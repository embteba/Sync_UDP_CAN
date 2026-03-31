// main.cpp
// UDPSender の簡易テスト

#include "UDPSender.hpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "=== UDPSender Simple Test ===" << std::endl;

    // テスト 1: loopback アドレスへのインスタンス生成
    std::cout << "\n[Test 1] Creating UDPSender(127.0.0.1:12345)..." << std::endl;
    UDPSender sender("127.0.0.1", 12345);
    std::cout << "OK - Instance created" << std::endl;

    // テスト 2: 初期化
    std::cout << "\n[Test 2] Calling init()..." << std::endl;
    bool init_result = sender.init();
    if (init_result) {
        std::cout << "OK - init() succeeded" << std::endl;
    } else {
        std::cout << "FAIL - init() failed" << std::endl;
        return 1;
    }

    // テスト 3: 1回目の送信
    std::cout << "\n[Test 3] Sending payload (attempt 1)..." << std::endl;
    std::string payload1("Hello UDP");
    bool send_result1 = sender.send_once(payload1);
    if (send_result1) {
        std::cout << "OK - send_once() succeeded for: " << payload1 << std::endl;
    } else {
        std::cout << "FAIL - send_once() failed" << std::endl;
    }

    // テスト 4: 2回目の送信（複数送信確認）
    std::cout << "\n[Test 4] Sending payload (attempt 2)..." << std::endl;
    std::string payload2("Test Message");
    bool send_result2 = sender.send_once(payload2);
    if (send_result2) {
        std::cout << "OK - send_once() succeeded for: " << payload2 << std::endl;
    } else {
        std::cout << "FAIL - send_once() failed" << std::endl;
    }

    // テスト 5: 明示的にクローズ
    std::cout << "\n[Test 5] Calling close()..." << std::endl;
    sender.close();
    std::cout << "OK - close() called" << std::endl;

    // テスト 6: クローズ後の再初期化（RAII による再利用確認）
    std::cout << "\n[Test 6] Re-initializing after close()..." << std::endl;
    bool reinit_result = sender.init();
    if (reinit_result) {
        std::cout << "OK - Re-init succeeded" << std::endl;
        std::string payload3("Re-initialized");
        bool send_result3 = sender.send_once(payload3);
        if (send_result3) {
            std::cout << "OK - send_once() succeeded after re-init for: " << payload3 << std::endl;
        }
    } else {
        std::cout << "FAIL - Re-init failed" << std::endl;
    }

    // テスト 7: デストラクタ（RAII の自動クリーンアップ）
    std::cout << "\n[Test 7] Destructor (auto cleanup)..." << std::endl;
    {
        UDPSender temp_sender("127.0.0.1", 54321);
        temp_sender.init();
        std::cout << "OK - Temporary instance created and initialized" << std::endl;
    }
    std::cout << "OK - Temporary instance destroyed (auto cleanup)" << std::endl;

    std::cout << "\n=== All Tests Completed ===" << std::endl;
    return 0;
}
