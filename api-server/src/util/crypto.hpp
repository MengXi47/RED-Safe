//
// Created by boen on 25-5-31.
//

#ifndef REDSAFE_UTIL_BASE64_HPP
#define REDSAFE_UTIL_BASE64_HPP

#include <sodium.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>

namespace redsafe::apiserver::util
{
    /**
    * @brief 把 input 內的二進位資料編成 Base64 (URL-safe, no padding) 字串
    *
    * @param input 任意 std::string，裡面可含二進位或純文字
    * @return std::string 回傳 Base64 編碼後的字串
    * @throws std::runtime_error 如果編碼失敗
    */
    inline std::string toBase64(const std::string &input)
    {
        // 第一步：確保 libsodium 已初始化
        if (sodium_init() < 0)
        {
            throw std::runtime_error("libsodium 初始化失敗");
        }

        // 計算對應的 Base64 目的緩衝區大小
        // sodium_base64_encoded_len(len, variant) 會回傳包含結尾 '\0' 的最小緩衝區長度
        size_t raw_len = input.size();
        size_t b64_buf_len = sodium_base64_encoded_len(raw_len, sodium_base64_VARIANT_URLSAFE_NO_PADDING);

        // 配置一個足夠大的緩衝區
        std::vector<char> b64buf(b64_buf_len);

        const char *written = sodium_bin2base64(
            b64buf.data(),
            b64_buf_len,
            reinterpret_cast<const unsigned char *>(input.data()),
            raw_len,
            sodium_base64_VARIANT_URLSAFE_NO_PADDING
        );

        if (written == nullptr)
        {
            throw std::runtime_error("Base64 編碼失敗");
        }

        // b64buf 裡已經包含 NUL 結尾（'\0'），直接用 data() 建 std::string
        return {b64buf.data()};
    }

    static std::string loadOrGenerateSecret()
    {
        // 先確保 libsodium 已初始化
        if (sodium_init() < 0)
            throw std::runtime_error("libsodium 初始化失敗");

        static auto SECRET_FILE_PATH = "jwt_secret.txt";

        // 1. 嘗試讀取已存在的金鑰
        if (std::ifstream infile(SECRET_FILE_PATH); infile.is_open())
        {
            std::string existingB64;
            std::getline(infile, existingB64);
            infile.close();
            if (!existingB64.empty())
                return existingB64;
        }

        // 2. 如果檔案不存在或讀到空，就重新產生一組 32 bytes 隨機資料
        constexpr size_t KEY_LEN = 32; // 256 位元
        unsigned char buf[KEY_LEN];
        randombytes_buf(buf, KEY_LEN); // 產生 KEY_LEN bytes 隨機

        // 把隨機位元組轉成 std::string
        std::string rawKey(reinterpret_cast<char *>(buf), KEY_LEN);

        std::string b64 = toBase64(rawKey);
        std::ofstream outfile(SECRET_FILE_PATH, std::ios::out | std::ios::trunc);
        if (!outfile.is_open())
            throw std::runtime_error("無法寫入金鑰檔案: " + std::string(SECRET_FILE_PATH));
        outfile << b64;
        outfile.close();

        return b64;
    }
}

#endif
