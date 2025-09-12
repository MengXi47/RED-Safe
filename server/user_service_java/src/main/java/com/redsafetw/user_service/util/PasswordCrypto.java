package com.redsafetw.user_service.util;

import org.springframework.security.crypto.argon2.Argon2PasswordEncoder;

/**
 * 密碼雜湊與驗證工具 Argon2id
 *
 * @create 2025-09-11 10:35 pm
 */
public final class PasswordCrypto {
    // 參數建議值（可依硬體調整）：
    private static final int SALT_LENGTH = 16;     // bytes
    private static final int HASH_LENGTH = 32;     // bytes
    private static final int PARALLELISM = Math.max(2, Runtime.getRuntime().availableProcessors() / 2);
    private static final int MEMORY_KB = 65536;    // 64 MB
    private static final int ITERATIONS = 3;       // 反覆次數

    private static final Argon2PasswordEncoder ENCODER = new Argon2PasswordEncoder(
            SALT_LENGTH,
            HASH_LENGTH,
            PARALLELISM,
            MEMORY_KB,
            ITERATIONS
    );

    private PasswordCrypto() {
    }

    /**
     * 產生雜湊
     */
    public static String hash(CharSequence rawPassword) {
        return ENCODER.encode(rawPassword);
    }

    /**
     * 驗證密碼是否匹配既有雜湊
     */
    public static boolean verify(String encodedPassword, CharSequence rawPassword) {
        if (encodedPassword == null || rawPassword == null) return false;
        return ENCODER.matches(rawPassword, encodedPassword);
    }
}

