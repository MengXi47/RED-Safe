package com.redsafetw.user_service.service;

import com.redsafetw.user_service.dto.*;
import com.redsafetw.user_service.domain.UserDomain;
import com.redsafetw.user_service.util.*;
import com.redsafetw.user_service.repository.UserRepository;
import com.redsafetw.user_service.repository.AuthRepository;
import com.redsafetw.user_service.domain.AuthDomain;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.springframework.stereotype.Service;
import org.springframework.http.HttpStatus;
import org.springframework.web.server.ResponseStatusException;

import java.time.OffsetDateTime;


/**
 * 使用者服務
 *
 * @create 2025-09-11 11:49 pm
 */
@Service
@Transactional
@RequiredArgsConstructor
public class UserService {
    private final UserRepository users;
    private final AuthRepository auths;

    public SignupResponse signup(SignupRequest req) {
        // 基本檢查
        if (req == null || req.getEmail() == null || req.getPassword() == null || req.getUser_name() == null) {
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "缺少參數");
        }

        // 驗證Email使否重複
        if (users.existsByEmail(req.getEmail())) {
            throw new ResponseStatusException(HttpStatus.CONFLICT, "Email already registered");
        }

        // 寫入資料庫
        UserDomain user = new UserDomain();
        user.setEmail(req.getEmail());
        user.setUser_name(req.getUser_name());
        user.setUser_password_hash(Argon2id.hash(req.getPassword()));
        user.setStatus(true);
        user = users.save(user);

        return SignupResponse.builder()
                .user_id(user.getUser_id())
                .user_name(user.getUser_name())
                .build();
    }

    public SigninResponse signin(SigninRequest req) {
        // 基本檢查
        if (req == null || req.getEmail() == null || req.getPassword() == null) {
            throw new ResponseStatusException(HttpStatus.BAD_REQUEST, "Email / password 不可為空");
        }

        // 以 Email 取得使用者
        var userOpt = users.findByEmail(req.getEmail().trim());
        if (userOpt.isEmpty()) {
            // 不揭露是帳號還是密碼錯，統一回 401
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "Invalid credentials");
        }

        var user = userOpt.get();

        // 驗證密碼
        boolean ok = Argon2id.verify(user.getUser_password_hash(), req.getPassword());
        if (!ok) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "FUCK PASSWORD INCORRECT");
        }

        String accessToken = JWT.createToken(user.getUser_id());
        String refreshToken = RefreshToken.generateRefreshToken();

        // 更新使用者最後登入時間（使用已載入的持久化實體）
        user.setLast_login_at(OffsetDateTime.now());
        users.save(user); // 在 @Transactional 作用下，save 或自動 flush 皆可

        AuthDomain auth = new AuthDomain();
        auth.setUser(user);
        auth.setRefresh_token(refreshToken);
        auth.setCreated_at(OffsetDateTime.now());
        auth.setExpires_at(OffsetDateTime.now().plusDays(30));
        auth.setRevoked(false);
        auths.save(auth);

        return SigninResponse.builder()
                .user_name(user.getUser_name())
                .access_token(accessToken)
                .refresh_token(refreshToken)
                .build();
    }

}