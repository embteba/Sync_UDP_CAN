// UDPSender.cpp
// Windows 専用 UDP 送信実装
// - エラー処理はプロジェクト方針に従い `// TODO: handle error` を残す

#include "UDPSender.hpp"

#include <cstring>
// winsock ヘッダは UDPSender.hpp 経由でインクルード済み

// コンストラクタ: ソケット初期化は init() で行う
UDPSender::UDPSender(const std::string& dest_ip, unsigned short dest_port)
    : dest_ip_(dest_ip), dest_port_(dest_port), sock_(INVALID_SOCKET), dest_addr_len_(0), initialized_(false) {
}

// デストラクタ: ソケットをクローズしてリソースを解放する
UDPSender::~UDPSender() {
    close_socket();
}

// init: winsock とソケットを初期化する
bool UDPSender::init() {
    if (initialized_) return true;
    init_socket();
    initialized_ = (sock_ != INVALID_SOCKET);
    return initialized_;
}

// send_once: ペイロードを送信する
bool UDPSender::send_once(const std::string& payload) {
    if (!initialized_) {
        if (!init()) return false;
    }

    const char* data = payload.data();
    int len = static_cast<int>(payload.size());
    // sendto は winsock API
    int sent = sendto(sock_, data, len, 0,
                      reinterpret_cast<struct sockaddr*>(&dest_addr_), dest_addr_len_);
    if (sent == SOCKET_ERROR) {
        // TODO: handle error
        return false;
    }
    return true;
}

void UDPSender::close() {
    close_socket();
    initialized_ = false;
}

// init_socket: winsock 初期化とソケット生成、宛先解決
void UDPSender::init_socket() {

    // winsock の初期化
    WSADATA wsa = {};
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        // TODO: handle error
    }

    // UDP ソケット生成（winsock の socket）
    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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
    if (sock_ != INVALID_SOCKET) {
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
    }
    WSACleanup();
}
