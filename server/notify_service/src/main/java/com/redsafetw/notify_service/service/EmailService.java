package com.redsafetw.notify_service.service;

import jakarta.mail.MessagingException;
import jakarta.mail.internet.MimeMessage;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.mail.SimpleMailMessage;
import org.springframework.mail.javamail.JavaMailSender;
import org.springframework.mail.javamail.MimeMessageHelper;
import org.springframework.stereotype.Service;

import java.io.File;
import java.nio.charset.StandardCharsets;

@Service
@RequiredArgsConstructor
@Slf4j
public class EmailService {

    private static final String DEFAULT_FROM = "RED-SAFE <noreply@redsafe-tw.com>";

    private final JavaMailSender mailSender;
    private final EmailLogService emailLogService;

    public void sendText(String to, String subject, String body) {
        SimpleMailMessage msg = new SimpleMailMessage();
        msg.setFrom(DEFAULT_FROM);
        msg.setTo(to);
        msg.setSubject(subject);
        msg.setText(body);

        try {
            mailSender.send(msg);
            emailLogService.log(DEFAULT_FROM, to, subject, body, true, null);
        } catch (RuntimeException ex) {
            emailLogService.log(DEFAULT_FROM, to, subject, body, false, ex.getMessage());
            throw ex;
        }
    }

    public void sendHtml(String to, String subject, String html) throws MessagingException {
        MimeMessage mime = mailSender.createMimeMessage();
        try {
            MimeMessageHelper helper = new MimeMessageHelper(
                    mime,
                    MimeMessageHelper.MULTIPART_MODE_MIXED_RELATED,
                    StandardCharsets.UTF_8.name()
            );
            helper.setFrom(DEFAULT_FROM);
            helper.setTo(to);
            helper.setSubject(subject);
            helper.setText(html, true);

            mailSender.send(mime);
            emailLogService.log(DEFAULT_FROM, to, subject, html, true, null);
        } catch (MessagingException | RuntimeException ex) {
            emailLogService.log(DEFAULT_FROM, to, subject, html, false, ex.getMessage());
            throw ex;
        }
    }

    public void sendEmailVerifyCode(String to, String code, int ttlMinutes) throws MessagingException {
        String subject = "驗證您的電子郵件地址";
        String html = """
                <!doctype html><html lang="zh-Hant"><head>
                  <meta charset="utf-8">
                  <meta name="viewport" content="width=device-width,initial-scale=1">
                  <title>RED-SAFE 驗證碼</title>
                  <style>
                    body{margin:0;background:#f5f6f8;font-family:system-ui,-apple-system,"Segoe UI",Roboto,"Noto Sans TC",Arial;}
                    .wrap{max-width:640px;margin:0 auto;padding:24px;}
                    .card{background:#fff;border-radius:10px;box-shadow:0 1px 4px rgba(0,0,0,.06);overflow:hidden;}
                    .brand{text-align:center;padding:24px 24px 16px;border-bottom:1px solid #eee;}
                    .brand-logo{width:120px;height:120px;border-radius:5px;display:block;margin:0 auto 8px;}
                    .brand-name{font-weight:700;font-size:24px;display:block;color:#111;}
                    .content{padding:24px}
                    .title{font-size:16px;color:#111;margin:0 0 6px;text-align:center;}
                    .code{font-size:40px;letter-spacing:.12em;font-weight:800;margin:12px 0 8px;text-align:center;}
                    .hint{color:#666;font-size:13px;margin:0 0 16px;text-align:center;}
                    .footer{color:#8a8a8a;font-size:12px;padding:18px 24px;border-top:1px solid #eee;text-align:center;}
                  </style>
                </head><body>
                  <div class="wrap">
                    <div class="card">
                      <div class="brand">
                        <img src="cid:logoImage" alt="logo" class="brand-logo"/>
                        <span class="brand-name">RED-SAFE</span>
                      </div>
                      <div class="content">
                        <p class="title">驗證碼</p>
                        <div class="code">%s</div>
                        <p class="hint">此驗證碼將在 %d 分鐘後失效。</p>
                        <p style="color:#444;font-size:14px;line-height:1.7;text-align:center;">
                          為保障您的帳號安全，請勿將驗證碼提供給他人。如果您未發起此操作，建議忽略本郵件。
                        </p>
                      </div>
                      <div class="footer">
                        &copy; RED-SAFE — 這封郵件由系統自動發送，請勿直接回覆。
                      </div>
                    </div>
                  </div>
                </body></html>
                """.formatted(code, ttlMinutes);

        MimeMessage mime = mailSender.createMimeMessage();
        try {
            MimeMessageHelper helper = new MimeMessageHelper(
                    mime,
                    MimeMessageHelper.MULTIPART_MODE_RELATED,
                    StandardCharsets.UTF_8.name()
            );

            helper.setFrom(DEFAULT_FROM);
            helper.setTo(to);
            helper.setSubject(subject);
            helper.setText(html, true);
//
//            File logoFile = new File(System.getProperty("user.home") + "/Desktop/RED_Safe_icon1.png");
//            helper.addInline("logoImage", logoFile);

            String logoPath = System.getenv().getOrDefault("LOGO_PATH", "/app/assets/RED_Safe_icon1.png");
            File logoFile = new File(logoPath);
            helper.addInline("logoImage", logoFile);

            String body = "MailVerify: " + code;
            mailSender.send(mime);
            emailLogService.log(DEFAULT_FROM, to, subject, html, true, null);
        } catch (MessagingException | RuntimeException ex) {
            emailLogService.log(DEFAULT_FROM, to, subject, html, false, ex.getMessage());
            throw ex;
        }
    }
}
