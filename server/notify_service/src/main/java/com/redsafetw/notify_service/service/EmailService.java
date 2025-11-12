package com.redsafetw.notify_service.service;

import jakarta.mail.MessagingException;
import jakarta.mail.internet.MimeMessage;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.core.io.ByteArrayResource;
import org.springframework.mail.SimpleMailMessage;
import org.springframework.mail.javamail.JavaMailSender;
import org.springframework.mail.javamail.MimeMessageHelper;
import org.springframework.stereotype.Service;
import org.springframework.util.StringUtils;

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

    /**
     * 發送跌倒通知郵件，並根據需要插入現場截圖。
     *
     * @param to                 收件者電子郵件
     * @param patientId          裝置/病患識別碼
     * @param ipAddress          IPC IP 位址
     * @param ipcName            IPC 顯示名稱
     * @param eventTime          事件發生時間（字串）
     * @param location           事件位置描述
     * @param snapshotImageBytes 現場截圖二進位內容，若為 null 或空陣列則不附圖
     * @param snapshotMimeType   截圖 Content-Type，預設 image/jpeg
     *                           
     */
    public void sendFallAlert(
            String to,
            String patientId,
            String ipAddress,
            String ipcName,
            String eventTime,
            String location,
            byte[] snapshotImageBytes,
            String snapshotMimeType
    ) throws MessagingException {
        // 判斷是否真的有截圖要附加，避免無意義的 multipart 郵件
        boolean hasSnapshot = snapshotImageBytes != null && snapshotImageBytes.length > 0;
        String contentType = StringUtils.hasText(snapshotMimeType) ? snapshotMimeType : "image/jpeg";

        String subject = "RED-SAFE 跌倒緊急通報";
        // 依據有無截圖決定 HTML 區塊（避免重複字串拼接）
        String snapshotSection = hasSnapshot
                ? """
                <div class="snapshot">
                  <p>系統擷取到當下畫面：</p>
                  <img src="cid:fallSnapshot" alt="跌倒事件截圖"/>
                </div>
                """
                : """
                <div class="snapshot placeholder">
                  <p>目前無可用的畫面，請儘速登入系統確認即時影像。</p>
                </div>
                """;
        String html = """
                <!doctype html>
                <html lang="zh-Hant"><head>
                  <meta charset="utf-8">
                  <meta name="viewport" content="width=device-width,initial-scale=1">
                  <title>跌倒緊急通報</title>
                  <style>
                    body{margin:0;background:#f5f6f8;font-family:system-ui,-apple-system,"Segoe UI",Roboto,"Noto Sans TC",Arial;}
                    .wrap{max-width:640px;margin:0 auto;padding:24px;}
                    .card{background:#fff;border-radius:12px;box-shadow:0 6px 20px rgba(16,24,40,.08);overflow:hidden;}
                    .header{background:#d92d20;color:#fff;padding:20px 24px;}
                    .header h1{margin:0;font-size:22px;font-weight:700;letter-spacing:.04em;}
                    .content{padding:24px 24px 16px;color:#101828;}
                    .content p{margin:0 0 16px;font-size:15px;line-height:1.6;}
                    .badge{display:inline-block;background:#fee4e2;color:#b42318;border-radius:6px;padding:4px 10px;font-size:12px;font-weight:600;margin-bottom:16px;}
                    .info{border-top:1px solid #f2f4f7;padding:16px 24px;background:#f9fafb;}
                    .info dt{font-size:12px;font-weight:600;color:#475467;margin:0 0 4px;text-transform:uppercase;letter-spacing:.08em;}
                    .info dd{margin:0 0 16px;font-size:15px;color:#1d2939;}
                    .snapshot{padding:0 24px 24px;}
                    .snapshot img{display:block;width:100%%;max-height:420px;object-fit:cover;border-radius:10px;box-shadow:0 4px 18px rgba(15,23,42,.2);}
                    .snapshot.placeholder{color:#475467;font-size:14px;}
                    .footer{font-size:12px;color:#98a2b3;padding:18px 24px;background:#101828;color:#98a2b3;text-align:center;}
                  </style>
                </head><body>
                  <div class="wrap">
                    <div class="card">
                      <div class="header">
                        <h1>跌倒事件通報</h1>
                      </div>
                      <div class="content">
                        <span class="badge">需要即時關注</span>
                        <p>系統偵測到您的監測裝置發生跌倒事件，請即時確認當事人狀況。詳細資訊如下：</p>
                      </div>
                      <div class="info">
                        <dl>
                          <dt>裝置 ID</dt>
                          <dd>%s</dd>
                          <dt>攝影機 IP</dt>
                          <dd>%s</dd>
                          <dt>攝影機名稱</dt>
                          <dd>%s</dd>
                          <dt>發生時間</dt>
                          <dd>%s</dd>
                          <dt>發生地點</dt>
                          <dd>%s</dd>
                        </dl>
                      </div>
                      %s
                      <div class="footer">
                        本郵件由 RED-SAFE 系統自動發送
                      </div>
                    </div>
                  </div>
                </body></html>
                """.formatted(patientId, ipAddress, ipcName, eventTime, location, snapshotSection);

        MimeMessage mime = mailSender.createMimeMessage();
        try {
            // 使用 MULTIPART_MODE_RELATED 以便插入 inline 圖片
            MimeMessageHelper helper = new MimeMessageHelper(
                    mime,
                    hasSnapshot,
                    StandardCharsets.UTF_8.name()
            );
            helper.setFrom(DEFAULT_FROM);
            helper.setTo(to);
            helper.setSubject(subject);
            helper.setText(html, true);

            // 只有真的有截圖時才建立 inline 附檔，避免空附件
            if (hasSnapshot) {
                ByteArrayResource snapshotResource = new ByteArrayResource(snapshotImageBytes);
                helper.addInline("fallSnapshot", snapshotResource, contentType);
            }

            mailSender.send(mime);
            emailLogService.log(DEFAULT_FROM, to, subject, html, true, null);
        } catch (MessagingException | RuntimeException ex) {
            emailLogService.log(DEFAULT_FROM, to, subject, html, false, ex.getMessage());
            throw ex;
        }
    }

    public void sendFallAlert(
            String to,
            String patientId,
            String ipAddress,
            String ipcName,
            String eventTime,
            String location
    ) throws MessagingException {
        sendFallAlert(to, patientId, ipAddress, ipcName, eventTime, location, null, null);
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
