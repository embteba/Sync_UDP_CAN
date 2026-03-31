// UDPSender.cpp
// Windows 専用 UDP 送信実装
// - エラー処理はプロジェクト方針に従い `// TODO: handle error` を残す

#include "UDPSender.hpp"

#include <cstring>
#include <ws2tcpip.h>
// getaddrinfo / freeaddrinfo を利用するため ws2tcpip.h を明示的にインクルード

// コンストラクタ: 宛先 IP とポートを保持し、ソケット初期化は init() まで遅延する
UDPSender::UDPSender(const std::string& dest_ip, unsigned short dest_port)
    : dest_ip_(dest_ip), dest_port_(dest_port), sock_(INVALID_SOCKET), dest_addr_len_(0), initialized_(false) {
}

// デストラクタ: ソケットをクローズしてリソースを解放する
UDPSender::~UDPSender() {
    close_socket();
}

// 初期化済みでない場合は winsock とソケットを初期化する。失敗時は false を返す。
bool UDPSender::init() {
    if (initialized_) return true;
    init_socket();
    initialized_ = (sock_ != INVALID_SOCKET);
    return initialized_;
}

// ペイロードを 1 回だけ UDP で送信する。未初期化なら自動で init() を試みる。
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

// 明示的にソケットをクローズし、initialized_ フラグをリセットする
void UDPSender::close() {
    close_socket();
    initialized_ = false;
}

// winsock の初期化の初期化、UDP ソケット生成、宛先アドレスの設定を一括で行う
void UDPSender::init_socket() {

    // winsock の初期化
    WSADATA wsa = {};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        // TODO: handle error
    }

    // UDP ソケット生成（winsock の socket）
    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET) {
        // TODO: handle error
    }

    // 宛先アドレスを sockaddr_in に設定（IPv4 のみ対応）
    struct sockaddr_in addr4;
    std::memset(&addr4, 0, sizeof(addr4));
    addr4.sin_family = AF_INET;
    addr4.sin_port = htons(dest_port_);

    // IPv4 アドレス文字列を in_addr に変換（inet_addr を使用）
    addr4.sin_addr.s_addr = inet_addr(dest_ip_.c_str());
    if (addr4.sin_addr.s_addr == INADDR_NONE) {
        // TODO: handle error
    }

    std::memset(&dest_addr_, 0, sizeof(dest_addr_));
    std::memcpy(&dest_addr_, &addr4, sizeof(addr4));
    dest_addr_len_ = static_cast<int>(sizeof(addr4));
}

// ソケットハンドルを閉じ、winsock のリソースを解放する
void UDPSender::close_socket() {
    if (sock_ != INVALID_SOCKET) {
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
    }
    WSACleanup();
}
