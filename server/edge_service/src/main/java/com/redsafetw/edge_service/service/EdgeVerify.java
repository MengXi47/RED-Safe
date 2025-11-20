package com.redsafetw.edge_service.service;

import com.redsafetw.edge_service.repository.EdgeRepository;
import com.redsafetw.edge_service.util.Argon2id;
import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.web.server.ResponseStatusException;

/**
 * 邊緣裝置驗證服務
 */
@Service
@Transactional
@RequiredArgsConstructor
public class EdgeVerify {
    private final EdgeRepository edgeRepository;

    /**
     * 驗證邊緣裝置是否存在且密碼匹配
     *
     * @param edgeId      邊緣裝置 ID
     * @param rawPassword 明文密碼
     * @return 存在且密碼正確返回 true，否則返回 false
     */
    public boolean verifyCredentials(String edgeId, String rawPassword) {
        if (edgeId == null || rawPassword == null) {
            return false;
        }

        return edgeRepository.findByEdgeId(edgeId)
                .map(device -> Argon2id.verify(device.getEdgePasswordHash(), rawPassword))
                .orElse(false);
    }

    /**
     * 更新邊緣裝置密碼
     *
     * @param edgeId         邊緣裝置 ID
     * @param currentPassword 目前密碼
     * @param newPassword    新密碼
     */
    public void updatePassword(String edgeId, String currentPassword, String newPassword) {
        var device = edgeRepository.findByEdgeId(edgeId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND, "123"));

        boolean matches = Argon2id.verify(device.getEdgePasswordHash(), currentPassword);
        if (!matches) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "147");
        }

        String hashedPassword = Argon2id.hash(newPassword);
        device.setEdgePasswordHash(hashedPassword);
        edgeRepository.save(device);
    }
}
