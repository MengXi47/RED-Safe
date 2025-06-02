/******************************************************************************
Copyright (C) 2025 by CHEN,BO-EN <chenboen931204@gmail.com>. All Rights Reserved.

   This file and its contents are proprietary and confidential.
   Unauthorized reproduction, distribution, or modification is strictly prohibited.

   Without the prior written permission of CHEN,BO-EN , you may not:
     1. Modify, adapt, or create derivative works of this source code;
     2. Reverse engineer, decompile, or otherwise attempt to derive the source code;
     3. Distribute, display, or otherwise use this source code or its derivatives in any form.

   For licensing inquiries or to obtain a formal license, please contact:
******************************************************************************/

#ifndef REDSAFE_CRYPTO_UTIL_HPP
#define REDSAFE_CRYPTO_UTIL_HPP

#include <sodium.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <cstdio>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include "../../config.hpp"

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
        const size_t raw_len = input.size();
        const size_t b64_buf_len = sodium_base64_encoded_len(raw_len, sodium_base64_VARIANT_URLSAFE_NO_PADDING);

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

    /**
     * @brief 將 Base64 字串解碼回原始 std::string
     *
     * @param input Base64 編碼字串
     * @return std::string 解碼後的原始二進位資料
     * @throws std::runtime_error 如果解碼過程失敗
     */
    inline std::string fromBase64(const std::string &input)
    {
        // 確保 libsodium 已初始化
        if (sodium_init() < 0)
            throw std::runtime_error("libsodium 初始化失敗");

        // 估算解碼後最大長度：Base64 長度 * 3/4 + 1
        size_t input_len = input.size();
        size_t raw_maxlen = input_len * 3 / 4 + 1;

        // 配置足夠的緩衝區
        std::vector<unsigned char> rawbuf(raw_maxlen);
        size_t raw_len = 0;

        // 呼叫 sodium_base642bin 進行解碼
        if (sodium_base642bin(
                rawbuf.data(),
                raw_maxlen,
                input.c_str(),
                input_len,
                nullptr,       // 不忽略任何字元
                &raw_len,
                nullptr,       // 不需要錯誤位置
                sodium_base64_VARIANT_URLSAFE_NO_PADDING) != 0)
            throw std::runtime_error("Base64 解碼失敗");

        // 回傳解碼後的 std::string
        return {reinterpret_cast<char*>(rawbuf.data()), raw_len};
    }

    /**
     * @brief 將原始二進位資料轉換為十六進位字串 (每 byte 兩個 hex 字元)
     *
     * @param data 指向要編碼的原始二進位資料
     * @param len  原始資料長度（單位：bytes）
     * @return std::string 編碼後的十六進位字串 (長度 = len * 2)
     */
    static std::string toHex(const unsigned char *data, const size_t len)
    {
        static constexpr char hex[] = "0123456789ABCDEF";
        std::string out;
        out.reserve(len * 2);
        for (size_t i = 0; i < len; ++i)
        {
            out.push_back(hex[data[i] >> 4]);
            out.push_back(hex[data[i] & 0xF]);
        }
        return out;
    }

    /**
     * @brief 將十六進位編碼字串還原為原始二進位資料
     *
     * @param hexStr 包含偶數個 0-9、A-F 或 a-f 字元的十六進位字串
     * @return std::vector<unsigned char> 解碼後的原始二進位資料 (長度 = hexStr.size() / 2)
     * @throws std::runtime_error 若 hexStr 長度非偶數或包含無效 hex 字元
     */
    static std::vector<unsigned char> fromHex(const std::string &hexStr)
    {
        if (hexStr.size() % 2)
            throw std::runtime_error("Hex 字串長度必須為偶數");

        auto hexVal = [](const char ch) -> int
        {
            if ('0' <= ch && ch <= '9') return ch - '0';
            if ('A' <= ch && ch <= 'F') return ch - 'A' + 10;
            if ('a' <= ch && ch <= 'f') return ch - 'a' + 10;
            return -1;
        };

        std::vector<unsigned char> out(hexStr.size() / 2);
        for (size_t i = 0; i < out.size(); ++i)
        {
            const int hi = hexVal(hexStr[2 * i]);
            const int lo = hexVal(hexStr[2 * i + 1]);
            if (hi < 0 || lo < 0)
                throw std::runtime_error("Hex 字元無效");
            out[i] = static_cast<unsigned char>((hi << 4) | lo);
        }
        return out;
    }

    /**
     * @brief 管理 JWT 簽章金鑰，只執行一次載入或生成
     */
    class SecretManager
    {
    public:
        /**
         * @brief 取得單例實例
         * Thread-Safe
         * @return SecretManager& 單例物件
         */
        static SecretManager& instance()
        {
            static SecretManager inst;
            return inst;
        }

        /**
         * @brief 取得 Base64 編碼後的 JWT 簽章金鑰
         * @return const std::string& 金鑰字串
         */
        [[nodiscard]] const std::string& getSecret() const
        {
            return secret_;
        }

        // 禁止複製與移動
        SecretManager(const SecretManager &) = delete;
        SecretManager &operator=(const SecretManager &) = delete;
        SecretManager(SecretManager &&) = delete;
        SecretManager &operator=(SecretManager &&) = delete;

    private:
        std::string secret_;

        // 私有建構式：嘗試載入或生成金鑰
        SecretManager()
        {
            // 確保 libsodium 已初始化
            if (sodium_init() < 0)
                throw std::runtime_error("libsodium 初始化失敗");

            // 嘗試從檔案載入已存在的金鑰
            if (std::ifstream infile(SECRET_FILE_PATH, std::ios::binary); infile.is_open())
            {
                std::string existingB64;
                std::getline(infile, existingB64);
                infile.close();
                if (!existingB64.empty())
                {
                    secret_ = existingB64;
                    return;
                }
            }

            // 檔案不存在或為空，生成新的 32 字節隨機金鑰
            constexpr size_t KEY_LEN = 32; // 256 位元
            unsigned char buf[KEY_LEN];
            randombytes_buf(buf, KEY_LEN); // 產生 KEY_LEN bytes 隨機

            // 將隨機位元組轉成 Base64
            std::string rawKey(reinterpret_cast<char*>(buf), KEY_LEN);
            std::string b64 = toBase64(rawKey);

            // 將 Base64 字串寫入檔案
            std::ofstream outfile(SECRET_FILE_PATH, std::ios::out | std::ios::trunc);
            if (!outfile.is_open())
                throw std::runtime_error("無法寫入金鑰檔案: " + std::string(SECRET_FILE_PATH));
            outfile << b64;
            outfile.close();

            secret_ = std::move(b64);
        }
    };

    /**
    * @brief 管理 AES-256-CBC 金鑰，只在第一次存取時讀取 AES_KEY.txt
    */
    class AESManager
    {
    public:
        /// 取得 thread-safe 單例
        static AESManager &instance()
        {
            static AESManager inst;
            return inst;
        }

        /**
         * @brief 加密 – 回傳十六進位字串（IV‖Ciphertext 轉 hex）
         * @param plaintext 明文
         * @return std::string hex-cipher
         */
        [[nodiscard]] std::string encrypt(const std::string &plaintext) const
        {
            // 1. 產生隨機 IV
            unsigned char iv[AES_IV_LEN];
            if (RAND_bytes(iv, AES_IV_LEN) != 1)
                throw std::runtime_error("RAND_bytes 生成 IV 失敗");

            // 2. 取得 thread‑local 的加密 context（只配置一次，之後重複使用）
            thread_local EVP_CIPHER_CTX *ctx = []() {
                EVP_CIPHER_CTX *p = EVP_CIPHER_CTX_new();
                if (!p) throw std::runtime_error("EVP_CIPHER_CTX_new 失敗 (加密)");
                return p;
            }();
            // 清空舊狀態，準備新一輪加密
            EVP_CIPHER_CTX_reset(ctx);

            if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                                   reinterpret_cast<const unsigned char *>(key_.data()),
                                   iv
                ) != 1)
            {
                throw std::runtime_error("EVP_EncryptInit_ex 失敗");
            }

            // 3. 執行加密
            std::vector<unsigned char> cipherBuf(plaintext.size() + AES_BLOCK_SIZE);
            int len1 = 0, len2 = 0;
            const auto pt_size = plaintext.size();
            if (pt_size > static_cast<std::size_t>(INT_MAX))
            {
                throw std::runtime_error("明文過長，超過 OpenSSL int 限制");
            }

            if (EVP_EncryptUpdate(ctx, cipherBuf.data(), &len1,
                                  reinterpret_cast<const unsigned char *>(plaintext.data()),
                                  static_cast<int>(pt_size)
                ) != 1 ||
                EVP_EncryptFinal_ex(ctx, cipherBuf.data() + len1, &len2) != 1)
            {
                throw std::runtime_error("EVP_EncryptUpdate/Final 失敗");
            }

            // 4. 組成 IV‖Ciphertext
            const size_t cipherLen = len1 + len2;
            std::vector<unsigned char> combined(AES_IV_LEN + cipherLen);
            std::memcpy(combined.data(), iv, AES_IV_LEN);
            std::memcpy(combined.data() + AES_IV_LEN, cipherBuf.data(), cipherLen);

            return toHex(combined.data(), combined.size());
        }

        /**
         * @brief 解密 – 參數為 encrypt() 產生的 hex 字串
         * @param hexCipher IV ‖ Ciphertext（十六進位）
         * @return std::string 明文
         */
        [[nodiscard]] std::string decrypt(const std::string &hexCipher) const
        {
            const std::vector<unsigned char> combined = fromHex(hexCipher);
            if (combined.size() < AES_IV_LEN)
                throw std::runtime_error("密文長度錯誤：缺少 IV");

            const auto cipher_sz = combined.size() - AES_IV_LEN;
            if (cipher_sz > static_cast<std::size_t>(INT_MAX))
                throw std::runtime_error("密文過長，超過 OpenSSL int 限制");

            const int cipherLen = static_cast<int>(cipher_sz);
            const unsigned char *iv = combined.data();
            const unsigned char *cipher = iv + AES_IV_LEN;

            // 取得 thread‑local 的解密 context
            thread_local EVP_CIPHER_CTX *ctx = []() {
                EVP_CIPHER_CTX *p = EVP_CIPHER_CTX_new();
                if (!p) throw std::runtime_error("EVP_CIPHER_CTX_new 失敗 (解密)");
                return p;
            }();
            EVP_CIPHER_CTX_reset(ctx);

            if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                                   reinterpret_cast<const unsigned char *>(key_.data()),
                                   iv
                ) != 1)
            {
                throw std::runtime_error("EVP_DecryptInit_ex 失敗");
            }

            std::vector<unsigned char> plainBuf(cipherLen);
            int len1 = 0, len2 = 0;

            if (EVP_DecryptUpdate(ctx, plainBuf.data(), &len1,
                                  cipher, cipherLen
                ) != 1 ||
                EVP_DecryptFinal_ex(ctx, plainBuf.data() + len1, &len2) != 1)
            {
                throw std::runtime_error("EVP_DecryptUpdate/Final 失敗 (金鑰或資料錯誤)");
            }

            return {reinterpret_cast<char *>(plainBuf.data()), static_cast<std::string::size_type>(len1 + len2)};
        }

        // 禁止拷貝/移動
        AESManager(const AESManager &) = delete;
        AESManager &operator=(const AESManager &) = delete;
        AESManager(AESManager &&) = delete;
        AESManager &operator=(AESManager &&) = delete;

    private:
        static constexpr size_t AES_KEY_LEN = 32; // 256 bits
        static constexpr size_t AES_IV_LEN = 16; // 128-bit IV

        std::string key_; // raw 32 bytes

        // 產生隨機 32‑byte 金鑰並寫入檔案（hex），然後存入 key_
        void generateAndSaveKey()
        {
            unsigned char buf[AES_KEY_LEN];
            if (RAND_bytes(buf, AES_KEY_LEN) != 1)
                throw std::runtime_error("RAND_bytes 生成 AES key 失敗");

            key_.assign(reinterpret_cast<char*>(buf), AES_KEY_LEN);

            // 將金鑰轉為 hex 字串
            const std::string hexKey = toHex(reinterpret_cast<unsigned char*>(buf), AES_KEY_LEN);

            // 把 hex 字串寫到 AES_KEY.txt (文字模式)
            std::ofstream out(AES_KEY_FILE_PATH, std::ios::out | std::ios::trunc);
            if (!out.is_open())
                throw std::runtime_error("無法建立 AES_KEY.txt 以寫入金鑰");
            out << hexKey;
            out.close();
        }

        /// 單例建構：只讀一次金鑰
        AESManager()
        {
            // 嘗試讀取 AES_KEY.txt
            std::ifstream in(AES_KEY_FILE_PATH, std::ios::binary);
            if (!in.is_open())
            {
                // 檔案不存在：直接生成並寫檔 (hex)
                generateAndSaveKey();
                return;
            }

            std::string buf((std::istreambuf_iterator<char>(in)),
                             std::istreambuf_iterator<char>());
            in.close();

            // 移除空白與換行
            std::erase_if(buf,
                          [](const char c){ return c=='\n'||c=='\r'||c==' '; });

            // 僅接受 64 個 hex 字元
            if (buf.size() == AES_KEY_LEN * 2)
            {
                std::vector<unsigned char> raw = fromHex(buf);
                if (raw.size() == AES_KEY_LEN)
                {
                    key_.assign(reinterpret_cast<char*>(raw.data()), AES_KEY_LEN);
                    return;
                }
            }

            // 若到此代表檔案不合法，刪除後重建 (hex)
            std::remove(AES_KEY_FILE_PATH);
            generateAndSaveKey();
       }
    };

    /**
    * @brief 產生一個隨機 hex 字串
    *
    * @param byteLen 要產生多少原始隨機位元組（例如 32 = 256 bit）
    * @return std::string 回傳的 hex 字串長度就是 byteLen * 2
    * @throws std::runtime_error 如果 libsodium 初始化失敗
    */
    [[nodiscard]] inline std::string generateRandomHex(const std::size_t byteLen)
    {
        if (sodium_init() < 0)
            throw std::runtime_error("libsodium 初始化失敗");

        std::vector<unsigned char> buf(byteLen);
        randombytes_buf(buf.data(), byteLen);


        const std::size_t hexLen = byteLen * 2 + 1;
        std::vector<char> hexBuf(hexLen);
        sodium_bin2hex(
            hexBuf.data(),
            hexLen,
            buf.data(),
            byteLen
        );

        return {hexBuf.data()};
    }

    /**
     * @brief  將明文雜湊
     * @param  str  明文
     * @return Argon2id 雜湊字串
     */
    [[nodiscard]] inline std::string HASH(std::string_view str)
    {
        if (sodium_init() < 0)
            throw std::runtime_error("Failed to initialize password hashing library");
        std::vector<char> hashBuf(crypto_pwhash_STRBYTES);
        if (crypto_pwhash_str(
                hashBuf.data(),
                str.data(), str.size(),
                crypto_pwhash_OPSLIMIT_INTERACTIVE,
                crypto_pwhash_MEMLIMIT_INTERACTIVE
            ) != 0)
            throw std::runtime_error("Password hashing failed");
        return {hashBuf.data()};
    }

    /**
     * @brief  驗證明文密碼是否與儲存雜湊相符
     * @param  hash   Argon2id 雜湊字串
     * @param  str  明文
     * @return true = 正確；false = 不符
     */
    [[nodiscard]] inline bool VerifyHASH(const std::string &hash, const std::string_view str)
    {
        if (sodium_init() < 0)
            throw std::runtime_error("Failed to initialize sodium");
        return crypto_pwhash_str_verify(
                   hash.c_str(),
                   str.data(),
                   str.size()
               ) == 0;
    }

    /**
     * @brief 以 SHA‑256 產生固定十六進位雜湊；同字串每次結果都一致
     *
     * @param input 任意明文字串
     * @return std::string 64 個大寫 hex 字元
     */
    [[nodiscard]] inline std::string SHA256_HEX(const std::string_view input)
    {
        unsigned char digest[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(input.data()),
               input.size(),
               digest);
        return toHex(digest, SHA256_DIGEST_LENGTH);
    }
}

#endif