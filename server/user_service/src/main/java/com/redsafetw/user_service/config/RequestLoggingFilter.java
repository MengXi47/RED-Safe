package com.redsafetw.user_service.config;

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

@Slf4j
@Component
public class RequestLoggingFilter extends OncePerRequestFilter {

    @Override
    protected void doFilterInternal(HttpServletRequest request,
                                    HttpServletResponse response,
                                    FilterChain filterChain) throws ServletException, IOException {
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
