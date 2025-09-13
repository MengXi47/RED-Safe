package com.redsafetw.user_service.util;

import io.jsonwebtoken.Jwts;
import io.jsonwebtoken.security.Keys;

import java.security.Key;
import java.time.OffsetDateTime;
import java.util.Date;
import java.util.UUID;

/**
 * JWT 工具
 *
 * @create 2025-09-11 10:35 pm
 */
public abstract class JWT {
    private static final Key key = Keys.secretKeyFor(io.jsonwebtoken.SignatureAlgorithm.HS256);

    /**
     * 發行 JWT，sub = user_id，過期時間 = 現在 + 10 分鐘
     *
     * @param userId 使用者 ID
     */
    public static String createToken(UUID userId) {
        OffsetDateTime now = OffsetDateTime.now();
        OffsetDateTime expiresAt = now.plusMinutes(10);
        return Jwts.builder()
                .setSubject(userId.toString())
                .setIssuedAt(new Date())
                .setExpiration(Date.from(expiresAt.toInstant()))
                .signWith(key)
                .compact();
    }

    /**
     * 驗證 JWT 並取得 userId
     *
     * @param token JWT 字串
     * @return userId 當驗證失敗或過期時 空 UUID (00000000-0000-0000-0000-000000000000)
     */
    public static UUID verifyAndGetUserId(String token) {
        try {
            return UUID.fromString(
                    Jwts.parserBuilder()
                            .setSigningKey(key)
                            .build()
                            .parseClaimsJws(token)
                            .getBody()
                            .getSubject()
            );
        } catch (Exception e) {
            return new UUID(0L, 0L); // 回傳空 user_id
        }
    }
}
