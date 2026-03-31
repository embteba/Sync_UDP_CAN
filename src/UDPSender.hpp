// UDPSender.hpp
// C++17、RAII ベースの UDP 送信クラス。指定したアドレスへ周期的にパケットを送信します。

#pragma once

#include <string>
#include <chrono>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#error "UDPSender is Windows-only"
#endif
class UDPSender {
public:
    /**
     * コンストラクタ: 宛先 IP とポートを指定して構築します。
     *
     * この最小実装はスレッドを使わず、1 回送信する `send_once()` を提供します。
     * ソケット初期化は内部で遅延して行われます。
     *
     * @param dest_ip 送信先の IPv4 アドレスまたはホスト名（例: "127.0.0.1"）。
     * @param dest_port 送信先のポート番号（例: 12345）。
     */
    UDPSender(const std::string& dest_ip, unsigned short dest_port);

    // デストラクタ: ソケットをクローズしてリソースを解放する
    ~UDPSender();

    // 初期化済みでない場合は内部で初期化を試みる。失敗時は false を返す。
    bool init();

    // ペイロードを 1 回だけ送信する。成功すれば true を返す。
    // （バックグラウンドスレッドは使わない最小実装）
    bool send_once(const std::string& payload);

    // 明示的にソケットをクローズする
    void close();

private:
    void init_socket();
    void close_socket();

    // 送信先の IPv4 アドレスまたはホスト名
    std::string dest_ip_;
    // 送信先のポート番号
    unsigned short dest_port_;
    // winsock のソケットハンドル（INVALID_SOCKET で未初期化を表す）
    SOCKET sock_ = INVALID_SOCKET;
    // 送信先のアドレス情報（IPv4 の sockaddr_in を格納）
    struct sockaddr_storage dest_addr_;
    // dest_addr_ の有効サイズ（バイト数）
    int dest_addr_len_ = 0;
    // init_socket() が成功済みかどうかを示すフラグ
    bool initialized_ = false;
};
