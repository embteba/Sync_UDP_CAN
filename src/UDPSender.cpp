// UDPSender.cpp
// 最小限の RAII ベース UDP 送信実装。エラー処理箇所はプロジェクト方針に従い
// `// TODO: handle error` を残してあります。

#include "UDPSender.hpp"

#include <cstring>
#include <system_error>
#include <chrono>
#include <iostream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#endif

UDPSender::UDPSender(const std::string& dest_ip, unsigned short dest_port, std::chrono::milliseconds period)
// Windows-only implementation (YAGNI). winsock headers are included in the header.
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
        // already running; payload replaced
        return;
    }

    init_socket();

    worker_ = std::thread(&UDPSender::run_loop, this);
}

void UDPSender::stop() {
    if (!running_.exchange(false)) return;
    if (worker_.joinable()) worker_.join();
}

bool UDPSender::is_running() const {
    return running_.load();
}

void UDPSender::init_socket() {
    std::lock_guard<std::mutex> lk(socket_mutex_);

#ifdef _WIN32
    WSADATA wsa = {};
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        // TODO: handle error
    }
#endif

    // UDP ソケットを作成
#ifdef _WIN32
    sock_ = static_cast<socket_t>(::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    if (sock_ == static_cast<socket_t>(INVALID_SOCKET)) {
        // TODO: handle error
    }
#else
    sock_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ < 0) {
        // TODO: handle error
    }
#endif

    // 宛先を名前解決
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4 only for simplicity
    hints.ai_socktype = SOCK_DGRAM;

    struct addrinfo* res = nullptr;
    int rv = getaddrinfo(dest_ip_.c_str(), nullptr, &hints, &res);
    if (rv != 0 || res == nullptr) {
        // TODO: handle error
        return;
    }

    // 宛先情報を `dest_addr_` に設定（IPv4）
    if (res->ai_family == AF_INET) {
        struct sockaddr_in addr4;
        std::memcpy(&addr4, res->ai_addr, res->ai_addrlen);
        addr4.sin_port = htons(dest_port_);
        std::memset(&dest_addr_, 0, sizeof(dest_addr_));
        std::memcpy(&dest_addr_, &addr4, sizeof(addr4));
        dest_addr_len_ = sizeof(addr4);
    } else {
        // TODO: handle error (non-IPv4 not supported in this minimal impl)
    }

    freeaddrinfo(res);
}

void UDPSender::close_socket() {
    std::lock_guard<std::mutex> lk(socket_mutex_);

#ifdef _WIN32
    if (sock_) {
        ::closesocket(static_cast<SOCKET>(sock_));
        sock_ = 0;
    }
    WSACleanup();
#else
    if (sock_ > 0) {
        ::close(sock_);
        sock_ = 0;
    }
#endif
}

void UDPSender::run_loop() {
    // ドリフトを避けるため steady_clock を使用して周期を管理します。
    auto next = std::chrono::steady_clock::now();
    while (running_.load()) {
        next += period_;

        // ペイロードを送信
        {
            std::lock_guard<std::mutex> lk(socket_mutex_);
            if (sock_ == 0) {
                // ソケットが利用できない状態
                // TODO: handle error
            } else {
                const char* data = payload_.data();
                int len = static_cast<int>(payload_.size());
#ifdef _WIN32
                int sent = ::sendto(static_cast<SOCKET>(sock_), data, len, 0,
                                     reinterpret_cast<struct sockaddr*>(&dest_addr_), static_cast<int>(dest_addr_len_));
                if (sent == SOCKET_ERROR) {
                    // TODO: handle error
                }
#else
                ssize_t sent = ::sendto(sock_, data, len, 0,
                                        reinterpret_cast<struct sockaddr*>(&dest_addr_), dest_addr_len_);
                if (sent < 0) {
                    // TODO: handle error
                }
#endif
            }
        }

        std::this_thread::sleep_until(next);
    }
}
