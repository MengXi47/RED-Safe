package com.redsafetw.auth_service.util;

import io.jsonwebtoken.Jwts;
import io.jsonwebtoken.security.Keys;

import java.security.Key;
import java.time.OffsetDateTime;
import java.util.Date;
import java.util.UUID;

public final class JwtService {

    private static final Key KEY = Keys.secretKeyFor(io.jsonwebtoken.SignatureAlgorithm.HS256);

    private JwtService() {
    }

    public static String createToken(UUID userId) {
        OffsetDateTime now = OffsetDateTime.now();
        OffsetDateTime expiresAt = now.plusMinutes(10);
        return Jwts.builder()
                .setSubject(userId.toString())
                .setIssuedAt(new Date())
                .setExpiration(Date.from(expiresAt.toInstant()))
                .signWith(KEY)
                .compact();
    }

    public static UUID verifyAndGetUserId(String token) {
        try {
            return UUID.fromString(
                    Jwts.parserBuilder()
                            .setSigningKey(KEY)
                            .build()
                            .parseClaimsJws(token)
                            .getBody()
                            .getSubject()
            );
        } catch (Exception ex) {
            return new UUID(0L, 0L);
        }
    }
}
