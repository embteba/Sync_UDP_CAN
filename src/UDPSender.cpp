// UDPSender.cpp
// Windows 専用の RAII ベース UDP 送信実装
// - winsock を用いて UDP ソケットを生成し、バックグラウンドスレッドで周期送信を行う
// - エラー処理はプロジェクト方針に従い `// TODO: handle error` を残す

#include "UDPSender.hpp"

#include <cstring>
#include <chrono>
#include <iostream>

// winsock ヘッダ
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

// コンストラクタ: 軽量に保ち、ソケット初期化は start() で行う
UDPSender::UDPSender(const std::string& dest_ip, unsigned short dest_port, std::chrono::milliseconds period)
    : dest_ip_(dest_ip), dest_port_(dest_port), period_(period), sock_(INVALID_SOCKET), dest_addr_len_(0) {
}

// デストラクタ: 送信停止とリソース解放を行う（RAII）
UDPSender::~UDPSender() {
    stop();
    close_socket();
}

// start: 指定ペイロードをバックグラウンドで周期送信する
void UDPSender::start(const std::string& payload) {
    payload_ = payload;
    if (running_.exchange(true)) {
        // 既に実行中の場合はペイロードを置き換えるだけ
        return;
    }

    init_socket();

    worker_ = std::thread(&UDPSender::run_loop, this);
}

// stop: 送信停止しスレッド終了を待つ
void UDPSender::stop() {
    if (!running_.exchange(false)) return;
    if (worker_.joinable()) worker_.join();
}

bool UDPSender::is_running() const {
    return running_.load();
}

// init_socket: winsock 初期化とソケット生成、宛先解決
void UDPSender::init_socket() {
    std::lock_guard<std::mutex> lk(socket_mutex_);

    // winsock の初期化
    WSADATA wsa = {};
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        // TODO: handle error
    }

    // UDP ソケット生成
    sock_ = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET) {
        // TODO: handle error
    }

    // 宛先を名前解決（簡潔化のため IPv4 のみ扱う）
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    struct addrinfo* res = nullptr;
    int rv = getaddrinfo(dest_ip_.c_str(), nullptr, &hints, &res);
    if (rv != 0 || res == nullptr) {
        // TODO: handle error
        return;
    }

    if (res->ai_family == AF_INET) {
        struct sockaddr_in addr4;
        std::memcpy(&addr4, res->ai_addr, res->ai_addrlen);
        addr4.sin_port = htons(dest_port_);
        std::memset(&dest_addr_, 0, sizeof(dest_addr_));
        std::memcpy(&dest_addr_, &addr4, sizeof(addr4));
        dest_addr_len_ = static_cast<int>(sizeof(addr4));
    } else {
        // 非 IPv4 は未対応
        // TODO: handle error
    }

    freeaddrinfo(res);
}

// close_socket: ソケットと winsock のクリーンアップ
void UDPSender::close_socket() {
    std::lock_guard<std::mutex> lk(socket_mutex_);

    if (sock_ != INVALID_SOCKET) {
        ::closesocket(sock_);
        sock_ = INVALID_SOCKET;
    }
    WSACleanup();
}

// run_loop: period_ 毎にペイロードを送信するループ
void UDPSender::run_loop() {
    // steady_clock を使ってドリフトを抑える
    auto next = std::chrono::steady_clock::now();
    while (running_.load()) {
        next += period_;

        // 送信処理: ソケットと宛先は mutex で保護
        {
            std::lock_guard<std::mutex> lk(socket_mutex_);
            if (sock_ == INVALID_SOCKET) {
                // ソケット未初期化や既にクローズされた状態
                // TODO: handle error
            } else {
                const char* data = payload_.data();
                int len = static_cast<int>(payload_.size());
                int sent = ::sendto(sock_, data, len, 0,
                                     reinterpret_cast<struct sockaddr*>(&dest_addr_), dest_addr_len_);
                if (sent == SOCKET_ERROR) {
                    // 送信失敗: ここでは詳細な再試行処理は行わない
                    // TODO: handle error
                }
            }
        }

        // 次送信時刻まで待機
        std::this_thread::sleep_until(next);
    }
}
