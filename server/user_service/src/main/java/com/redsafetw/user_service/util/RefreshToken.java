package com.redsafetw.user_service.util;

import java.security.SecureRandom;
import java.util.Base64;

/**
 * 刷新權杖建構器
 *
 * @create 2025-09-12
 **/
public class RefreshToken {
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final Base64.Encoder base64Encoder = Base64.getUrlEncoder().withoutPadding();

    public static String generateRefreshToken() {
        byte[] randomBytes = new byte[32]; // 256 bits
        secureRandom.nextBytes(randomBytes);
        return base64Encoder.encodeToString(randomBytes);
    }
}