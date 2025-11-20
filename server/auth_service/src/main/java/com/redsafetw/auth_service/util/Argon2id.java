package com.redsafetw.auth_service.util;

import lombok.extern.slf4j.Slf4j;
import org.springframework.security.crypto.argon2.Argon2PasswordEncoder;

@Slf4j
public final class Argon2id {

    private static final int SALT_LENGTH = 16;
    private static final int HASH_LENGTH = 32;
    private static final int PARALLELISM = Math.max(2, Runtime.getRuntime().availableProcessors() / 2);
    private static final int MEMORY_KB = 65536;
    private static final int ITERATIONS = 3;

    private static final Argon2PasswordEncoder ENCODER = new Argon2PasswordEncoder(
            SALT_LENGTH,
            HASH_LENGTH,
            PARALLELISM,
            MEMORY_KB,
            ITERATIONS
    );

    private Argon2id() {
    }

    public static String hash(CharSequence rawValue) {
        if (rawValue == null || rawValue.toString().trim().isEmpty()) {
            log.info("rawValue is null or empty");
        }
        return ENCODER.encode(rawValue);
    }

    public static boolean verify(String encodedValue, CharSequence rawValue) {
        if (encodedValue == null || rawValue == null) {
            return false;
        }
        return ENCODER.matches(rawValue, encodedValue);
    }
}
