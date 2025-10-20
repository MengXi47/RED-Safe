package com.redsafetw.auth_service.config;

import jakarta.servlet.FilterChain;
import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;
import org.springframework.web.filter.OncePerRequestFilter;
import org.springframework.web.util.ContentCachingRequestWrapper;
import org.springframework.web.util.ContentCachingResponseWrapper;

import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;

/**
 * 全域過濾器：記錄所有 HTTP 請求與回應的內容，協助除錯與審查。
 */
@Slf4j
@Component
public class RequestLoggingFilter extends OncePerRequestFilter {

    @Override
    protected void doFilterInternal(HttpServletRequest request,
                                    HttpServletResponse response,
                                    FilterChain filterChain) throws ServletException, IOException {
        if (isSseRequest(request)) {
            long start = System.currentTimeMillis();
            try {
                filterChain.doFilter(request, response);
            } finally {
                long duration = System.currentTimeMillis() - start;
                log.info("HTTP {} {} status={} duration={}ms (SSE stream)",
                        request.getMethod(), request.getRequestURI(), response.getStatus(), duration);
            }
            return;
        }
        long start = System.currentTimeMillis();
        ContentCachingRequestWrapper requestWrapper = new ContentCachingRequestWrapper(request);
        ContentCachingResponseWrapper responseWrapper = new ContentCachingResponseWrapper(response);
        try {
            filterChain.doFilter(requestWrapper, responseWrapper);
        } finally {
            long duration = System.currentTimeMillis() - start;
            String path = request.getRequestURI();
            String query = request.getQueryString();
            if (query != null && !query.isBlank()) {
                path = path + '?' + query;
            }
            String requestBody = bodyAsString(requestWrapper.getContentAsByteArray(), requestWrapper.getCharacterEncoding());
            String responseBody = bodyAsString(responseWrapper.getContentAsByteArray(), responseWrapper.getCharacterEncoding());
            log.info("HTTP {} {} status={} duration={}ms request_body={} response_body={}",
                    request.getMethod(), path, responseWrapper.getStatus(), duration, requestBody, responseBody);
            responseWrapper.copyBodyToResponse();
        }
    }

    private boolean isSseRequest(HttpServletRequest request) {
        String accept = request.getHeader("Accept");
        if (accept != null && accept.contains("text/event-stream")) {
            return true;
        }
        String requestUri = request.getRequestURI();
        return requestUri != null && requestUri.contains("/sse/");
    }

    private String bodyAsString(byte[] body, String encoding) {
        if (body == null || body.length == 0) {
            return "";
        }
        Charset charset = StandardCharsets.UTF_8;
        if (encoding != null && Charset.isSupported(encoding)) {
            charset = Charset.forName(encoding);
        }
        return new String(body, charset);
    }
}
