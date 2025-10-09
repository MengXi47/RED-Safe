package com.redsafetw.user_service.service;

import com.fasterxml.jackson.databind.JsonNode;
import com.redsafetw.user_service.repository.UserEdgeBindRepository;
import jakarta.annotation.PreDestroy;
import lombok.RequiredArgsConstructor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.web.servlet.mvc.method.annotation.SseEmitter;
import org.springframework.web.server.ResponseStatusException;

import java.io.IOException;
import java.time.Duration;
import java.util.Optional;
import java.util.UUID;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * SSE 取得 Edge 指令結果的服務。
 */
@Service
@RequiredArgsConstructor
public class EdgeCommandSseService {

    private static final Logger log = LoggerFactory.getLogger(EdgeCommandSseService.class);
    private static final Duration SSE_TIMEOUT = Duration.ofSeconds(15);
    private static final long POLL_INTERVAL_MS = 200L;

    private final EdgeCommandRedisService edgeCommandRedisService;
    private final UserEdgeBindRepository userEdgeBindRepository;
    private final ExecutorService emitterExecutor = Executors.newCachedThreadPool(r -> {
        Thread thread = new Thread(r, "edge-command-sse");
        thread.setDaemon(true);
        return thread;
    });

    /**
     * 建立即時事件串流並等待 Redis 更新。
     */
    public SseEmitter streamCommandResult(String traceId, String accessToken) {
        JsonNode requestNode = edgeCommandRedisService.getRequest(traceId)
                .orElseThrow(() -> new ResponseStatusException(HttpStatus.BAD_REQUEST, "155"));

        UUID userId = JwtService.verifyAndGetUserId(accessToken);
        if (userId.equals(new UUID(0L, 0L))) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "126");
        }

        String edgeId = requestNode.path("edge_id").asText();
        if (!userEdgeBindRepository.existsByUserIdAndEdgeId(userId, edgeId)) {
            throw new ResponseStatusException(HttpStatus.UNAUTHORIZED, "135");
        }

        SseEmitter emitter = new SseEmitter(SSE_TIMEOUT.plusSeconds(2).toMillis());
        emitter.onTimeout(emitter::complete);
        emitter.onError(ex -> log.warn("SSE error trace {}: {}", traceId, ex.getMessage()));

        emitterExecutor.submit(() -> emitResponse(traceId, emitter));
        return emitter;
    }

    /**
     * 輪詢 Redis 並將結果透過 SSE 傳出。
     */
    private void emitResponse(String traceId, SseEmitter emitter) {
        long deadline = System.currentTimeMillis() + SSE_TIMEOUT.toMillis();
        try {
            while (System.currentTimeMillis() < deadline) {
                Optional<JsonNode> response = edgeCommandRedisService.getResponse(traceId);
                if (response.isPresent()) {
                    sendPayload(traceId, emitter, response.get());
                    return;
                }
                Optional<String> raw = edgeCommandRedisService.getResponseRaw(traceId);
                if (raw.isPresent()) {
                    sendPayload(traceId, emitter, raw.get());
                    return;
                }
                Thread.sleep(POLL_INTERVAL_MS);
            }
            Optional<JsonNode> finalResponse = edgeCommandRedisService.getResponse(traceId);
            if (finalResponse.isPresent()) {
                sendPayload(traceId, emitter, finalResponse.get());
            } else {
                Optional<String> raw = edgeCommandRedisService.getResponseRaw(traceId);
                sendPayload(traceId, emitter, raw.orElse("notfound"));
            }
        } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
            safeCompleteWithError(emitter, ex);
        } catch (Exception ex) {
            safeCompleteWithError(emitter, ex);
        }
    }

    /**
     * 透過 SSE 傳送 payload 並結束連線。
     */
    private void sendPayload(String traceId, SseEmitter emitter, Object payload) throws IOException {
        Object dataToSend;
        if (payload instanceof JsonNode jsonNode) {
            dataToSend = jsonNode;
        } else if (payload instanceof String str) {
            dataToSend = str;
        } else {
            dataToSend = payload.toString();
        }
        emitter.send(SseEmitter.event()
                .id(traceId)
                .name("command")
                .data(dataToSend));
        emitter.complete();
    }

    /**
     * SSE 結束時記錄錯誤。
     */
    private void safeCompleteWithError(SseEmitter emitter, Exception ex) {
        log.warn("SSE streaming error", ex);
        emitter.completeWithError(ex);
    }

    /**
     * Bean 關閉時釋放資源。
     */
    @PreDestroy
    public void shutdown() {
        emitterExecutor.shutdownNow();
    }
}
