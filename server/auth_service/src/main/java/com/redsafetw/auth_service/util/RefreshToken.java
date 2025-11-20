package com.redsafetw.auth_service.util;

import java.security.SecureRandom;
import java.util.Base64;

public final class RefreshToken {

    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    private static final Base64.Encoder ENCODER = Base64.getUrlEncoder().withoutPadding();

    private RefreshToken() {
    }

    public static String generate() {
        byte[] randomBytes = new byte[32];
        SECURE_RANDOM.nextBytes(randomBytes);
        return ENCODER.encodeToString(randomBytes);
    }
}
