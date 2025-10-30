import SwiftUI
import UIKit

// MARK: - 帳號分頁

enum ProfileSheet: Identifiable {
    case displayName
    case password
    case otp

    var id: String {
        switch self {
        case .displayName: return "display-name"
        case .password: return "password"
        case .otp: return "otp"
        }
    }
}

/// AccountView 管理使用者帳號資訊與裝置註冊。
struct AccountView: View {
    @ObservedObject var auth: AuthManager
    @ObservedObject var profileVM: ProfileViewModel
    @Binding var profileSheet: ProfileSheet?
    @AppStorage("appAppearance") private var appearanceSelection = AppearanceMode.system.rawValue

    var body: some View {
        NavigationStack {
            ZStack {
                AppBackground()
                ScrollView(showsIndicators: false) {
                    VStack(spacing: 24) {
                        header
                        infoCard
                        signOutButton
                    }
                    .padding(.horizontal, 20)
                    .padding(.top, 18)
                    .padding(.bottom, 48)
                }
            }
            .sheet(item: $profileSheet) { sheet in
                switch sheet {
                case .displayName:
                    UpdateNameSheet(initialName: auth.userName ?? "") { newName, completion in
                        Task { @MainActor in
                            do {
                                _ = try await auth.updateUserName(to: newName)
                                profileVM.presentMessage("顯示名稱已更新")
                                completion(true)
                                profileSheet = nil
                            } catch {
                                profileVM.presentMessage(error.localizedDescription)
                                completion(false)
                            }
                        }
                    }
                case .password:
                    UpdatePasswordSheet { current, newPassword, completion in
                        Task { @MainActor in
                            let success = await profileVM.updatePassword(currentPassword: current, newPassword: newPassword)
                            if success { profileSheet = nil }
                            completion(success)
                        }
                    }
                case .otp:
                    OTPSetupIntroSheet(
                        viewModel: profileVM,
                        enabledInitial: auth.profile?.otpEnabled == true,
                        accountEmail: auth.profile?.email
                    )
                }
            }
            .navigationTitle("")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar(.hidden, for: .navigationBar)
            .toolbar(.visible, for: .tabBar)
            .task {
                await auth.refreshProfileFromRemote()
            }
        }
    }

    private var header: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("帳號與安全")
                .font(.system(size: 30, weight: .bold, design: .rounded))
                .foregroundStyle(Color.primary)
            Text("管理個人資料與通知設定")
                .font(.subheadline)
                .foregroundStyle(Color.secondary)
        }
        .frame(maxWidth: .infinity, alignment: .leading)
    }

    private var infoCard: some View {
        VStack(alignment: .leading, spacing: 16) {
            HStack(alignment: .top, spacing: 16) {
                ZStack {
                    Circle()
                        .fill(Color.blue.opacity(0.12))
                        .frame(width: 64, height: 64)
                    Text(initials(from: auth.profile?.displayName ?? auth.profile?.email ?? "R"))
                        .font(.title2.weight(.bold))
                        .foregroundColor(Color.blue)
                }
                VStack(alignment: .leading, spacing: 6) {
                    Text(auth.profile?.displayName ?? "尚未設定名稱")
                        .font(.headline)
                    Text(auth.profile?.email ?? "")
                        .font(.footnote)
                        .foregroundColor(.secondary)
                }
                Spacer()
            }

            Divider()

            VStack(alignment: .leading, spacing: 12) {
                Text("帳號設定")
                    .font(.subheadline.weight(.semibold))
                    .foregroundColor(.secondary)
                ButtonRow(icon: "pencil", title: "變更使用者名稱") {
                    profileSheet = .displayName
                }
                ButtonRow(icon: "lock.rotation", title: "變更使用者密碼") {
                    profileSheet = .password
                }
                otpStatusRow
                appearanceRow
            }
        }
        .padding(20)
        .frame(maxWidth: .infinity, alignment: .leading)
        .glassCard(cornerRadius: 28)
    }

    private var currentAppearance: AppearanceMode {
        AppearanceMode(rawValue: appearanceSelection) ?? .system
    }

    private var otpStatusRow: some View {
        let enabled = auth.profile?.otpEnabled == true
        return Button {
            profileSheet = .otp
        } label: {
            HStack {
                Label("二階段驗證 (OTP)", systemImage: "key.fill")
                    .labelStyle(.titleAndIcon)
                Spacer()
                Text(enabled ? "已啟用" : "未啟用")
                    .font(.caption.weight(.semibold))
                    .foregroundColor(enabled ? .green : .secondary)
                Image(systemName: "chevron.right")
                    .foregroundColor(Color(UIColor.tertiaryLabel))
            }
            .padding(.vertical, 10)
        }
        .buttonStyle(.plain)
    }

    private var appearanceRow: some View {
        let current = currentAppearance

        return Menu {
            ForEach(AppearanceMode.allCases) { mode in
                Button {
                    appearanceSelection = mode.rawValue
                } label: {
                    if mode == current {
                        Label(mode.displayName, systemImage: "checkmark")
                    } else {
                        Text(mode.displayName)
                    }
                }
            }
        } label: {
            HStack {
                HStack(spacing: 10) {
                    Image(systemName: "circle.lefthalf.fill")
                        .font(.body.weight(.semibold))
                        .foregroundStyle(Color.primary)
                    Text("介面模式")
                        .foregroundStyle(Color.primary)
                }
                Spacer()
                HStack(spacing: 6) {
                    Text(current.displayName)
                        .font(.caption.weight(.semibold))
                        .foregroundStyle(Color.secondary)
                    Image(systemName: "chevron.down")
                        .font(.caption.weight(.semibold))
                        .foregroundStyle(Color.secondary)
                }
            }
            .padding(.vertical, 10)
        }
    }

    private var signOutButton: some View {
        Button(role: .destructive) {
            auth.signOut()
        } label: {
            Label("登出", systemImage: "rectangle.portrait.and.arrow.right")
                .font(.headline)
                .frame(maxWidth: .infinity)
                .padding(.vertical, 14)
                .background(RoundedRectangle(cornerRadius: 18, style: .continuous).fill(Color.red.opacity(0.12)))
        }
        .buttonStyle(.plain)
    }

    private func initials(from text: String) -> String {
        let components = text.split(separator: " ")
        if let first = components.first, let char = first.first {
            return String(char).uppercased()
        }
        if let first = text.first {
            return String(first).uppercased()
        }
        return "R"
    }
}

// MARK: - 元件

private struct EnableOTPSheet: View {
    @Environment(\.dismiss) private var dismiss
    @ObservedObject var viewModel: ProfileViewModel

    @State private var otpKey: String?
    @State private var backupCodes: [String] = []
    @State private var isGenerating: Bool = false
    @State private var errorMessage: String?

    var body: some View {
        NavigationStack {
            ScrollView(showsIndicators: false) {
                VStack(alignment: .leading, spacing: 24) {
                    Text("啟用二階段驗證後，登入時需要輸入 6 碼 OTP 或備援碼。請妥善保存以下資訊，避免遺失。")
                        .font(.callout)
                        .foregroundColor(.secondary)

                    if let otpKey {
                        otpKeySection(key: otpKey)
                    }

                    if !backupCodes.isEmpty {
                        backupCodesSection
                    }

                    if let errorMessage {
                        Text(errorMessage)
                            .font(.footnote)
                            .foregroundColor(.red)
                    }

                    Button(action: { Task { await generateOTP() } }) {
                        Label(otpKey == nil ? "產生 OTP 金鑰" : "重新產生 OTP 金鑰", systemImage: "key.horizontal.fill")
                            .font(.headline)
                            .frame(maxWidth: .infinity)
                            .padding(.vertical, 14)
                    }
                    .disabled(isGenerating)
                    .buttonStyle(.borderedProminent)
                    .tint(.accentColor)

                    if isGenerating {
                        ProgressView("產生中…")
                            .frame(maxWidth: .infinity)
                    }
                }
                .padding(24)
            }
            .navigationTitle("啟用二階段驗證")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("關閉") { dismiss() }
                }
            }
            .task {
                if let existing = viewModel.lastOTPSetup, otpKey == nil {
                    otpKey = existing.otpKey
                    backupCodes = existing.backupCodes
                }
            }
        }
    }

    private func otpKeySection(key: String) -> some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("OTP 金鑰")
                .font(.subheadline.weight(.semibold))
                .foregroundColor(.secondary)
            Text(key)
                .font(.title3.monospaced())
                .padding()
                .frame(maxWidth: .infinity, alignment: .center)
                .background(RoundedRectangle(cornerRadius: 18, style: .continuous).fill(Color.surfaceBackground))
            Button(action: { copyToClipboard(key) }) {
                Label("複製金鑰", systemImage: "doc.on.doc")
                    .font(.subheadline.weight(.semibold))
                    .frame(maxWidth: .infinity)
            }
            .buttonStyle(.bordered)
        }
        .padding(20)
        .background(RoundedRectangle(cornerRadius: 24, style: .continuous).fill(Color.surfaceBackground))
        .overlay(RoundedRectangle(cornerRadius: 24, style: .continuous).stroke(Color.surfaceStroke))
    }

    private var backupCodesSection: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("備援驗證碼")
                .font(.subheadline.weight(.semibold))
                .foregroundColor(.secondary)
            Text("每組備援碼僅能使用一次，請存放於安全位置。")
                .font(.footnote)
                .foregroundColor(.secondary)

            ForEach(Array(backupCodes.enumerated()), id: \.offset) { index, code in
                HStack {
                    Text("#\(index + 1)")
                        .font(.caption)
                        .foregroundColor(.secondary)
                        .frame(width: 32, alignment: .leading)
                    Text(code)
                        .font(.body.monospaced())
                        .frame(maxWidth: .infinity, alignment: .leading)
                    Button(action: { copyToClipboard(code) }) {
                        Image(systemName: "doc.on.doc")
                            .foregroundColor(.accentColor)
                    }
                    .buttonStyle(.plain)
                }
                .padding(.vertical, 6)
                .padding(.horizontal, 12)
                .background(RoundedRectangle(cornerRadius: 16, style: .continuous).fill(Color.surfaceBackground))
            }
        }
        .padding(20)
        .background(RoundedRectangle(cornerRadius: 24, style: .continuous).fill(Color.surfaceBackground))
        .overlay(RoundedRectangle(cornerRadius: 24, style: .continuous).stroke(Color.surfaceStroke))
    }

    private func generateOTP() async {
        guard !isGenerating else { return }
        await MainActor.run {
            isGenerating = true
            errorMessage = nil
        }
        let response = await viewModel.enableOTP()
        await MainActor.run {
            isGenerating = false
            if let response {
                otpKey = response.otpKey
                backupCodes = response.backupCodes
                errorMessage = nil
            } else {
                errorMessage = viewModel.message
            }
        }
    }

    private func copyToClipboard(_ text: String) {
        UIPasteboard.general.string = text
    }
}

private struct OTPSetupIntroSheet: View {
    @Environment(\.dismiss) private var dismiss
    @ObservedObject var viewModel: ProfileViewModel
    let enabledInitial: Bool
    let accountEmail: String?

    @State private var showDisableConfirm = false
    @State private var isDisabling = false
    @State private var disableError: String?
    @State private var enabled: Bool = false
    @State private var isEnabling = false
    @State private var enableError: String?
    @State private var otpKey: String?
    @State private var backupCodes: [String] = []
    @State private var showQR: Bool = false

    var body: some View {
        NavigationStack {
            ScrollView(showsIndicators: false) {
                VStack(alignment: .leading, spacing: 24) {
                    Text("二階段驗證（OTP）能提升帳號安全性。啟用後，登入時需要輸入 6 碼一次性密碼或備援碼。")
                        .font(.callout)
                        .foregroundColor(.secondary)

                    VStack(alignment: .leading, spacing: 12) {
                        Label("需要一個認證器 App", systemImage: "app.badge.checkmark")
                        Text("例如：Google Authenticator、1Password、Authy 等。")
                            .font(.footnote)
                            .foregroundColor(.secondary)
                    }

                    Button {
                        if enabled {
                            showDisableConfirm = true
                        } else {
                            Task {
                                isEnabling = true
                                enableError = nil
                                let result = await viewModel.enableOTP()
                                isEnabling = false
                                if let result {
                                    enabled = true
                                    otpKey = result.otpKey
                                    backupCodes = result.backupCodes
                                    enableError = nil
                                } else {
                                    enableError = viewModel.message
                                }
                            }
                        }
                    } label: {
                        Label(enabled ? "停用 OTP" : "啟用 OTP", systemImage: enabled ? "key.slash.fill" : "key.horizontal.fill")
                            .font(.headline)
                            .frame(maxWidth: .infinity)
                            .padding(.vertical, 14)
                    }
                    .buttonStyle(.borderedProminent)
                    .tint(enabled ? .red : .accentColor)
                    .disabled(isEnabling || isDisabling)

                    if isEnabling {
                        ProgressView("正在啟用…")
                            .frame(maxWidth: .infinity, alignment: .leading)
                    }
                    if let enableError {
                        Text(enableError)
                            .font(.footnote)
                            .foregroundColor(.red)
                    }
                    if isDisabling {
                        ProgressView("正在停用…")
                            .frame(maxWidth: .infinity, alignment: .leading)
                    }
                    if let disableError {
                        Text(disableError)
                            .font(.footnote)
                            .foregroundColor(.red)
                    }

                    // --- Insert key, QR, backup codes sections here ---
                    if let key = otpKey {
                        otpKeySection(key: key)
                        if !showQR {
                            Button {
                                showQR = true
                            } label: {
                                Label("顯示 QRCode", systemImage: "qrcode")
                                    .font(.subheadline.weight(.semibold))
                                    .frame(maxWidth: .infinity)
                                    .padding(.vertical, 10)
                            }
                            .buttonStyle(.bordered)
                        } else if let qr = qrImage(for: key) {
                            VStack(alignment: .leading, spacing: 12) {
                                Text("掃描 QR Code 匯入")
                                    .font(.subheadline.weight(.semibold))
                                    .foregroundColor(.secondary)
                                Image(uiImage: qr)
                                    .interpolation(.none)
                                    .resizable()
                                    .scaledToFit()
                                    .frame(maxWidth: 240)
                                    .padding()
                                    .background(RoundedRectangle(cornerRadius: 18, style: .continuous).fill(Color.surfaceBackground))
                            }
                            .padding(20)
                            .background(RoundedRectangle(cornerRadius: 24, style: .continuous).fill(Color.surfaceBackground))
                            .overlay(RoundedRectangle(cornerRadius: 24, style: .continuous).stroke(Color.surfaceStroke))
                        }
                    }
                    if !backupCodes.isEmpty {
                        backupCodesSection
                    }
                }
                .padding(24)
            }
            .navigationTitle("二階段驗證")
            .navigationBarTitleDisplayMode(.inline)
            .task {
                enabled = enabledInitial
            }
            .alert("停用二階段驗證？", isPresented: $showDisableConfirm) {
                Button("停用", role: .destructive) {
                    Task {
                        isDisabling = true
                        disableError = nil
                        let success = await viewModel.disableOTP()
                        isDisabling = false
                        if success {
                            enabled = false
                            dismiss()
                        } else {
                            disableError = viewModel.message
                        }
                    }
                }
                Button("取消", role: .cancel) { }
            } message: {
                Text("停用後將清除一次性密碼與備援碼，登入時將不再需要輸入 OTP。")
            }
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("關閉") { dismiss() }
                }
            }
        }
    }

    private func otpKeySection(key: String) -> some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("OTP 金鑰")
                .font(.subheadline.weight(.semibold))
                .foregroundColor(.secondary)
            Text(key)
                .font(.title3.monospaced())
                .padding()
                .frame(maxWidth: .infinity, alignment: .center)
                .background(RoundedRectangle(cornerRadius: 18, style: .continuous).fill(Color.surfaceBackground))
            Button(action: { copyToClipboard(key) }) {
                Label("複製金鑰", systemImage: "doc.on.doc")
                    .font(.subheadline.weight(.semibold))
                    .frame(maxWidth: .infinity)
            }
            .buttonStyle(.bordered)
        }
        .padding(20)
        .background(RoundedRectangle(cornerRadius: 24, style: .continuous).fill(Color.surfaceBackground))
        .overlay(RoundedRectangle(cornerRadius: 24, style: .continuous).stroke(Color.surfaceStroke))
    }

    private var backupCodesSection: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("備援驗證碼")
                .font(.subheadline.weight(.semibold))
                .foregroundColor(.secondary)
            Text("每組備援碼僅能使用一次，請存放於安全位置。")
                .font(.footnote)
                .foregroundColor(.secondary)
            ForEach(Array(backupCodes.enumerated()), id: \.offset) { index, code in
                HStack {
                    Text("#\(index + 1)")
                        .font(.caption)
                        .foregroundColor(.secondary)
                        .frame(width: 32, alignment: .leading)
                    Text(code)
                        .font(.body.monospaced())
                        .frame(maxWidth: .infinity, alignment: .leading)
                    Button(action: { copyToClipboard(code) }) {
                        Image(systemName: "doc.on.doc")
                            .foregroundColor(.accentColor)
                    }
                    .buttonStyle(.plain)
                }
                .padding(.vertical, 6)
                .padding(.horizontal, 12)
                .background(RoundedRectangle(cornerRadius: 16, style: .continuous).fill(Color.surfaceBackground))
            }
        }
        .padding(20)
        .background(RoundedRectangle(cornerRadius: 24, style: .continuous).fill(Color.surfaceBackground))
        .overlay(RoundedRectangle(cornerRadius: 24, style: .continuous).stroke(Color.surfaceStroke))
    }

    private func qrImage(for secret: String) -> UIImage? {
        let issuer = "RED Safe"
        let account = accountEmail ?? "account"
        let label = "\(issuer):\(account)".addingPercentEncoding(withAllowedCharacters: .urlPathAllowed) ?? "\(issuer):\(account)"
        let query = "secret=\(secret)&issuer=\(issuer.addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed) ?? issuer)&algorithm=SHA1&digits=6&period=30"
        let otpauth = "otpauth://totp/\(label)?\(query)"
        guard let data = otpauth.data(using: .utf8) else { return nil }
        let filter = CIFilter(name: "CIQRCodeGenerator")
        filter?.setValue(data, forKey: "inputMessage")
        filter?.setValue("M", forKey: "inputCorrectionLevel")
        guard let outputImage = filter?.outputImage else { return nil }
        let transform = CGAffineTransform(scaleX: 8, y: 8)
        let scaled = outputImage.transformed(by: transform)
        let context = CIContext()
        if let cgimg = context.createCGImage(scaled, from: scaled.extent) {
            return UIImage(cgImage: cgimg)
        }
        return nil
    }

    private func copyToClipboard(_ text: String) {
        UIPasteboard.general.string = text
    }
}

private struct ButtonRow: View {
    let icon: String
    let title: String
    let action: () -> Void

    var body: some View {
        Button(action: action) {
            HStack {
                Label(title, systemImage: icon)
                    .labelStyle(.titleAndIcon)
                Spacer()
                Image(systemName: "chevron.right")
                    .foregroundColor(Color(UIColor.tertiaryLabel))
            }
            .padding(.vertical, 10)
        }
        .buttonStyle(.plain)
    }
}

// MARK: - 互動表單

private struct UpdateNameSheet: View {
    @Environment(\.dismiss) private var dismiss
    @State private var name: String

    let initialName: String
    let onSubmit: (String, @escaping (Bool) -> Void) -> Void

    init(initialName: String, onSubmit: @escaping (String, @escaping (Bool) -> Void) -> Void) {
        self.initialName = initialName
        self.onSubmit = onSubmit
        _name = State(initialValue: initialName)
    }

    private var trimmedName: String { name.trimmed }
    private var isValid: Bool { !trimmedName.isEmpty && trimmedName.count <= 16 }

    var body: some View {
        NavigationStack {
            Form {
                Section("顯示名稱") {
                    TextField("顯示名稱", text: $name)
                        .onChange(of: name) { value in
                            if value.count > 16 {
                                name = String(value.prefix(16))
                            }
                        }
                }
                if !isValid {
                    Text("名稱需為 1-16 字元")
                        .font(.footnote)
                        .foregroundColor(.red)
                }
            }
            .navigationTitle("變更顯示名稱")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("取消") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("儲存") {
                        onSubmit(trimmedName) { success in
                            if success { dismiss() }
                        }
                    }
                    .disabled(!isValid)
                }
            }
        }
    }
}

private struct UpdatePasswordSheet: View {
    @Environment(\.dismiss) private var dismiss
    @State private var currentPassword = ""
    @State private var newPassword = ""
    @State private var confirmPassword = ""
    @State private var isSubmitting = false

    let onSubmit: (String, String, @escaping (Bool) -> Void) -> Void

    private var isValid: Bool {
        !currentPassword.trimmed.isEmpty && !newPassword.trimmed.isEmpty && newPassword == confirmPassword
    }

    var body: some View {
        NavigationStack {
            Form {
                Section("修改密碼") {
                    SecureField("目前密碼", text: $currentPassword)
                    SecureField("新密碼", text: $newPassword)
                    SecureField("確認新密碼", text: $confirmPassword)
                }
                if newPassword != confirmPassword {
                    Text("兩次輸入的新密碼不同")
                        .font(.footnote)
                        .foregroundColor(.red)
                }
            }
            .navigationTitle("變更登入密碼")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("取消") { dismiss() }
                        .disabled(isSubmitting)
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("更新") {
                        isSubmitting = true
                        onSubmit(currentPassword.trimmed, newPassword.trimmed) { success in
                            isSubmitting = false
                            if success { dismiss() }
                        }
                    }
                    .disabled(!isValid || isSubmitting)
                }
            }
        }
    }
}
