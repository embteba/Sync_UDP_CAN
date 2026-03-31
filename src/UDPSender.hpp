// UDPSender.hpp
// C++17、RAII ベースの UDP 送信クラス。指定したアドレスへ周期的にパケットを送信します。

#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#error "This file is Windows-only per project YAGNI decision"
#endif

class UDPSender {
public:
    // デフォルト送信周期（ミリ秒）を定義。ハードコーディング防止のため定数化。
    static constexpr int DEFAULT_PERIOD_MS = 100;

    /**
     * 宛先 IP とポート、送信周期（デフォルト 100ms）を指定して構築します。
     *
     * @param dest_ip 送信先の IPv4 アドレスまたはホスト名（例: "127.0.0.1"）。
     * @param dest_port 送信先のポート番号（例: 12345）。
     * @param period 送信周期（単位: ミリ秒）。デフォルトは `std::chrono::milliseconds(100)`。
     *
     * 設計方針: コンストラクタは軽量に保ち、ソケット初期化は `start()` 実行時に遅延して行います。
     */
    UDPSender(const std::string& dest_ip, unsigned short dest_port,
              std::chrono::milliseconds period = std::chrono::milliseconds(DEFAULT_PERIOD_MS));

    // デストラクタ: 送信停止とリソース解放を行います（RAII）。
    ~UDPSender();

    // 指定したペイロードをバックグラウンドスレッドで周期的に送信します。
    // 既に動作中の場合はペイロードを置き換えます。
    void start(const std::string& payload);

    // 送信を停止し、スレッドの終了を待ちます。
    void stop();

    // 実行中状態を問い合わせます。
    bool is_running() const;

private:
    void run_loop();
    void init_socket();
    void close_socket();

    std::string dest_ip_;
    unsigned short dest_port_;
    std::chrono::milliseconds period_;
    std::string payload_;

    std::thread worker_;
    std::atomic<bool> running_{false};
    std::mutex socket_mutex_; // ソケットと宛先情報の同期保護

    SOCKET sock_ = INVALID_SOCKET;
    struct sockaddr_storage dest_addr_;
    int dest_addr_len_;
};
