package com.redsafetw.edge_service.util;

import org.springframework.security.crypto.argon2.Argon2PasswordEncoder;

/**
 * 雜湊與驗證工具 Argon2id
 *
 * @create 2025-09-11 10:35 pm
 */
public final class Argon2id {
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

    private Argon2id() {
    }

    /**
     * 產生雜湊
     */
    public static String hash(CharSequence rawValue) {
        return ENCODER.encode(rawValue);
    }

    /**
     * 驗證值是否匹配既有雜湊
     */
    public static boolean verify(String encodedValue, CharSequence rawValue) {
        if (encodedValue == null || rawValue == null) return false;
        return ENCODER.matches(rawValue, encodedValue);
    }
}
