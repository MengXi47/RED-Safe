import SwiftUI
import UIKit

/// SignInView 提供使用者登入介面，聚焦於輸入與回饋體驗。
struct SignInView: View {
    @StateObject private var viewModel = SignInViewModel()
    @FocusState private var focusedField: SignInViewModel.Field?
    @State private var animateBackground = false

    var body: some View {
        NavigationStack {
            ZStack {
                backgroundLayer
                ScrollView(showsIndicators: false) {
                    VStack(spacing: 36) {
                        heroSection
                        credentialCard
                        //featureHighlights
                    }
                    .padding(.horizontal, 24)
                    .padding(.bottom, 48)
                    .padding(.top, 32)
                }
            }
            .contentShape(Rectangle())
            .onTapGesture { focusedField = nil }
            .onAppear { animateBackground = true }
            .onChange(of: viewModel.banner) { banner in
                guard banner != nil else { return }
                UIImpactFeedbackGenerator(style: .soft).impactOccurred()
            }
            .onChange(of: focusedField) { oldValue, newValue in
                if oldValue == .email, newValue != .email {
                    viewModel.emailValidated = true
                }
            }
            .toolbar(.hidden, for: .navigationBar)
            .overlay(alignment: .top) {
                if let banner = viewModel.banner {
                    BannerView(banner: banner)
                        .padding(.top, 24)
                        .transition(.move(edge: .top).combined(with: .opacity))
                }
            }
            .overlay {
                if viewModel.isSubmitting {
                    ZStack {
                        Color.black.opacity(0.25).ignoresSafeArea()
                        ProgressView()
                            .progressViewStyle(.circular)
                            .tint(.white)
                            .scaleEffect(1.2)
                    }
                }
            }
            .animation(.spring(response: 0.45, dampingFraction: 0.85), value: viewModel.banner)
            .safeAreaInset(edge: .top) {
                Color.clear.frame(height: 4)
            }
            .sheet(item: $viewModel.pendingOTP) { pending in
                OTPVerificationSheet(viewModel: viewModel, pending: pending)
            }
        }
    }

    // MARK: - Sections

    private var backgroundLayer: some View {
        ZStack {
            LinearGradient(
                colors: [
                    Color(red: 244/255, green: 248/255, blue: 255/255),
                    Color(red: 224/255, green: 234/255, blue: 250/255),
                    Color(red: 204/255, green: 221/255, blue: 246/255)
                ],
                startPoint: .topLeading,
                endPoint: .bottomTrailing
            )
            .ignoresSafeArea()

            Circle()
                .fill(Color.white.opacity(0.5))
                .frame(width: animateBackground ? 360 : 220)
                .blur(radius: 60)
                .offset(x: -150, y: animateBackground ? -260 : -140)
                .animation(.easeOut(duration: 1.0), value: animateBackground)

            Circle()
                .fill(Color.white.opacity(0.35))
                .frame(width: animateBackground ? 320 : 200)
                .blur(radius: 48)
                .offset(x: 170, y: animateBackground ? 280 : 160)
                .animation(.easeOut(duration: 1.0).delay(0.05), value: animateBackground)
        }
    }

    private var heroSection: some View {
        VStack(spacing: 14) {
            Image("RED_Safe_icon1")
                .resizable()
                .scaledToFit()
                .frame(width: 160, height: 160)
                .shadow(color: Color.black.opacity(0.25), radius: 20, x: 0, y: 18)
                .scaleEffect(animateBackground ? 1 : 0.85)
                .opacity(animateBackground ? 1 : 0)
                .animation(.spring(response: 0.7, dampingFraction: 0.65).delay(0.05), value: animateBackground)

            VStack(spacing: 6) {
                Text("歡迎回到 RED Safe")
                    .font(.system(size: 32, weight: .bold, design: .rounded))
                    .foregroundStyle(Color.primary)
                    .multilineTextAlignment(.center)
                Text("登入以即時掌握家人的安全狀態")
                    .font(.callout)
                    .foregroundStyle(Color.secondary)
                    .multilineTextAlignment(.center)
            }
            .padding(.horizontal, 8)
        }
    }

    private var credentialCard: some View {
        VStack(spacing: 24) {
            VStack(spacing: 18) {
                CustomTextField(
                    title: "電子郵件",
                    text: $viewModel.email,
                    icon: "envelope.fill",
                    keyboard: .emailAddress,
                    isSecure: false,
                    isValid: !viewModel.emailValidated || viewModel.isEmailValid,
                    errorText: viewModel.emailErrorMessage
                )
                .focused($focusedField, equals: .email)
                .submitLabel(.next)
                .onSubmit {
                    focusedField = .password
                }

                CustomTextField(
                    title: "登入密碼",
                    text: $viewModel.password,
                    icon: "lock.fill",
                    keyboard: .default,
                    isSecure: !viewModel.isPasswordVisible,
                    isValid: viewModel.shouldShowPasswordError ? viewModel.isPasswordValid : true,
                    errorText: viewModel.passwordErrorMessage,
                    trailingIcon: viewModel.isPasswordVisible ? "eye.slash" : "eye",
                    trailingAction: { viewModel.isPasswordVisible.toggle() }
                )
                .focused($focusedField, equals: .password)
                .submitLabel(.done)
                .onSubmit { Task { await viewModel.submit() } }
            }

            Button(action: { Task { await viewModel.submit() } }) {
                ZStack {
                    RoundedRectangle(cornerRadius: 20, style: .continuous)
                        .fill(
                            LinearGradient(
                                colors: viewModel.canSubmit ? [
                                    Color(red: 118/255, green: 186/255, blue: 255/255),
                                    Color(red: 78/255, green: 156/255, blue: 255/255)
                                ] : [
                                    Color(UIColor.systemGray5),
                                    Color(UIColor.systemGray4)
                                ],
                                startPoint: .topLeading,
                                endPoint: .bottomTrailing
                            )
                        )
                    RoundedRectangle(cornerRadius: 20, style: .continuous)
                        .stroke(viewModel.canSubmit ? Color.white.opacity(0.4) : Color.black.opacity(0.08))
                    HStack(spacing: 12) {
                        if viewModel.isSubmitting {
                            ProgressView()
                                .progressViewStyle(.circular)
                                .tint(.white)
                        } else {
                            Image(systemName: "arrow.right.circle.fill")
                                .font(.title3)
                                .foregroundStyle(viewModel.canSubmit ? .white : .secondary)
                        }
                        Text(viewModel.isSubmitting ? "登入中" : "登入")
                            .font(.headline.weight(.semibold))
                            .foregroundColor(viewModel.canSubmit ? .white : .secondary)
                    }
                    .padding(.vertical, 16)
                }
                .frame(height: 60)
            }
            .buttonStyle(.plain)
            .disabled(!viewModel.canSubmit)
            .shadow(color: Color.black.opacity(viewModel.canSubmit ? 0.35 : 0.0), radius: 24, x: 0, y: 18)

            NavigationLink(destination: SignUpView()) {
                Text("還沒有帳號？立即註冊")
                    .font(.subheadline.weight(.semibold))
                    .foregroundColor(.accentColor)
                    .frame(maxWidth: .infinity)
                    .padding(.vertical, 14)
                    .glassCard(cornerRadius: 16, opacity: 0.95)
            }
            .buttonStyle(.plain)
        }
        .padding(24)
        .glassCard(cornerRadius: 32)
    }

//    private var featureHighlights: some View {
//        VStack(alignment: .leading, spacing: 18) {
//            Text("RED Safe 優勢")
//                .font(.title3.weight(.semibold))
//                .foregroundColor(.primary)
//
//            VStack(alignment: .leading, spacing: 14) {
//                FeatureRow(icon: "bell.badge.fill", title: "即時警示", detail: "當 Edge 裝置偵測到異常情況時立即收到通知")
//                FeatureRow(icon: "lock.shield", title: "企業級安全", detail: "Refresh Token 自動續期，登入狀態無縫保持")
//                FeatureRow(icon: "rectangle.connected.to.line.below", title: "全方位裝置管理", detail: "綁定、重命名及管理多台 Edge 裝置更直覺")
//            }
//            .padding(24)
//            .glassCard(cornerRadius: 28)
//        }
//    }
}

// MARK: - Components

private struct FeatureRow: View {
    let icon: String
    let title: String
    let detail: String

    var body: some View {
        HStack(alignment: .firstTextBaseline, spacing: 16) {
            Image(systemName: icon)
                .foregroundColor(.accentColor)
                .font(.title3)
                .frame(width: 32, height: 32)
            VStack(alignment: .leading, spacing: 4) {
                Text(title)
                    .font(.headline)
                    .foregroundColor(.primary)
                Text(detail)
                    .font(.footnote)
                    .foregroundColor(.secondary)
            }
            Spacer()
        }
    }
}

private struct CustomTextField: View {
    let title: String
    @Binding var text: String
    let icon: String
    let keyboard: UIKeyboardType
    let isSecure: Bool
    let isValid: Bool
    var errorText: String?
    var trailingIcon: String?
    var trailingAction: (() -> Void)?

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack(spacing: 14) {
                Image(systemName: icon)
                    .foregroundStyle(Color.accentColor.opacity(0.8))
                    .frame(width: 22, alignment: .center)
                if isSecure {
                    SecureField(title, text: $text)
                        .keyboardType(keyboard)
                        .textInputAutocapitalization(.never)
                        .autocorrectionDisabled()
                        .foregroundColor(.primary)
                } else {
                    TextField(title, text: $text)
                        .keyboardType(keyboard)
                        .textInputAutocapitalization(.never)
                        .autocorrectionDisabled()
                        .foregroundColor(.primary)
                }

                if let trailingIcon {
                    Button(action: { trailingAction?() }) {
                        Image(systemName: trailingIcon)
                            .foregroundStyle(Color.accentColor.opacity(0.8))
                    }
                    .buttonStyle(.plain)
                }
            }
            .padding(.vertical, 14)
            .padding(.horizontal, 16)
            .background(
                RoundedRectangle(cornerRadius: 18, style: .continuous)
                    .fill(Color.white.opacity(0.95))
            )
            .overlay(
                RoundedRectangle(cornerRadius: 18, style: .continuous)
                    .stroke(isValid ? Color.accentColor.opacity(0.25) : Color.red.opacity(0.7), lineWidth: 1.2)
            )

            if let errorText, !isValid {
                Text(errorText)
                    .font(.footnote)
                    .foregroundStyle(Color.red.opacity(0.85))
                    .transition(.opacity)
            }
        }
    }
}

private struct BannerView: View {
    let banner: SignInViewModel.Banner

    var body: some View {
        HStack(spacing: 12) {
            Image(systemName: banner.kind == .success ? "checkmark.circle.fill" : "exclamationmark.triangle.fill")
                .foregroundStyle(banner.kind == .success ? Color.green : Color.red)
                .font(.title3)
            VStack(alignment: .leading, spacing: 2) {
                Text(banner.title)
                    .font(.headline)
                Text(banner.message)
                    .font(.footnote)
            }
            .foregroundStyle(.primary)
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 12)
        .background(.thinMaterial, in: Capsule())
        .shadow(color: Color.black.opacity(0.2), radius: 18, x: 0, y: 12)
    }
}

private struct OTPVerificationSheet: View {
    enum Field: Hashable {
        case otp
    }

    @Environment(\.dismiss) private var dismiss
    @ObservedObject var viewModel: SignInViewModel
    let pending: SignInViewModel.PendingOTP

    @State private var otpCode: String = ""
    @State private var validationError: String?
    @FocusState private var focusedField: Field?

    var body: some View {
        NavigationStack {
            VStack(spacing: 20) {
                // Account chip
                HStack(spacing: 8) {
                    Image(systemName: "envelope.fill")
                        .foregroundColor(.accentColor)
                    Text(pending.email)
                        .font(.callout.monospaced())
                        .foregroundColor(.primary)
                }
                .padding(.vertical, 8)
                .padding(.horizontal, 14)
                .background(.thinMaterial, in: Capsule())
                .shadow(color: Color.black.opacity(0.08), radius: 10, x: 0, y: 8)
                .padding(.top, 8)

                // Title & hint
                VStack(spacing: 6) {
                    Text("輸入 6 碼 OTP")
                        .font(.title3.weight(.semibold))
                    Text("請開啟你的認證 App（Google Authenticator、1Password、Authy…）輸入目前顯示的 6 碼。")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                        .multilineTextAlignment(.center)
                        .padding(.horizontal, 24)
                }

                // Hidden field to capture input
                TextField("", text: $otpCode)
                    .keyboardType(.numberPad)
                    .textContentType(.oneTimeCode)
                    .textInputAutocapitalization(.never)
                    .autocorrectionDisabled()
                    .focused($focusedField, equals: .otp)
                    .onChange(of: otpCode) { otpCode = sanitize(code: $0) }
                    .opacity(0.02)
                    .frame(width: 1, height: 1)

                // Six-digit boxes
                HStack(spacing: 12) {
                    ForEach(0..<6, id: \.self) { idx in
                        let char: String = {
                            let array = Array(otpCode)
                            return idx < array.count ? String(array[idx]) : ""
                        }()

                        ZStack {
                            RoundedRectangle(cornerRadius: 14, style: .continuous)
                                .fill(Color.white.opacity(0.95))
                            RoundedRectangle(cornerRadius: 14, style: .continuous)
                                .stroke(
                                    idx == min(otpCode.count, 5) && focusedField == .otp
                                    ? Color.accentColor.opacity(0.6)
                                    : Color.black.opacity(0.08),
                                    lineWidth: 1.2
                                )
                            Text(char.isEmpty ? " " : char)
                                .font(.title2.monospacedDigit().weight(.semibold))
                                .foregroundColor(.primary)
                        }
                        .frame(width: 48, height: 56)
                        .contentShape(Rectangle())
                        .onTapGesture { focusedField = .otp }
                    }
                }
                .padding(.top, 4)

                if let validationError {
                    Text(validationError)
                        .font(.footnote)
                        .foregroundColor(.red)
                        .padding(.top, 4)
                }


                Button(action: { Task { await verifyOTP() } }) {
                    ZStack {
                        RoundedRectangle(cornerRadius: 18, style: .continuous)
                            .fill(
                                LinearGradient(
                                    colors: viewModel.isVerifyingOTP ? [
                                        Color(UIColor.systemGray5),
                                        Color(UIColor.systemGray4)
                                    ] : [
                                        Color(red: 118/255, green: 186/255, blue: 255/255),
                                        Color(red: 78/255, green: 156/255, blue: 255/255)
                                    ],
                                    startPoint: .topLeading,
                                    endPoint: .bottomTrailing
                                )
                            )
                        RoundedRectangle(cornerRadius: 18, style: .continuous)
                            .stroke(viewModel.isVerifyingOTP ? Color.black.opacity(0.08) : Color.white.opacity(0.4))
                        HStack(spacing: 10) {
                            if viewModel.isVerifyingOTP {
                                ProgressView().tint(.white)
                            } else {
                                Image(systemName: "checkmark.circle.fill")
                                    .font(.headline)
                                    .foregroundStyle(.white)
                            }
                            Text(viewModel.isVerifyingOTP ? "驗證中" : "送出驗證")
                                .font(.headline.weight(.semibold))
                                .foregroundColor(.white)
                        }
                        .padding(.vertical, 14)
                    }
                    .frame(height: 54)
                }
                .buttonStyle(.plain)
                .disabled(viewModel.isVerifyingOTP || otpCode.count != 6)
                .shadow(color: Color.black.opacity(otpCode.count == 6 ? 0.25 : 0.0), radius: 18, x: 0, y: 12)

                Spacer(minLength: 8)
            }
            .padding(.horizontal, 24)
            .padding(.vertical, 20)
            .navigationTitle("OTP 驗證")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("取消") {
                        viewModel.pendingOTP = nil
                        dismiss()
                    }
                }
            }
            .onAppear {
                focusedField = .otp
            }
        }
    }

    private func sanitize(code: String) -> String {
        let filtered = code.filter { $0.isNumber }
        return String(filtered.prefix(6))
    }

    private func verifyOTP() async {
        let trimmedOTP = otpCode.trimmingCharacters(in: .whitespacesAndNewlines)

        guard !trimmedOTP.isEmpty else {
            await MainActor.run { validationError = "請輸入 OTP 驗證碼" }
            return
        }

        await MainActor.run { validationError = nil }

        let result = await viewModel.completeOTP(otpCode: trimmedOTP)

        if case .success = result {
            dismiss()
        } else if case .failure(let error) = result {
            await MainActor.run {
                if let localized = (error as? LocalizedError)?.errorDescription {
                    validationError = localized
                } else {
                    validationError = error.localizedDescription
                }
            }
        }
    }
}

// MARK: - View Model

@MainActor
/// SignInViewModel 負責處理驗證與表單狀態，保持 View 簡潔 (SRP)。
final class SignInViewModel: ObservableObject {
    enum Field: Hashable {
        case email
        case password
    }

    struct Banner: Identifiable, Equatable {
        enum Kind { case success, error }
        let id = UUID()
        let title: String
        let message: String
        let kind: Kind
    }

    struct PendingOTP: Identifiable, Equatable {
        let id = UUID()
        let email: String
        let password: String
    }

    enum OTPFlowError: LocalizedError {
        case missingContext

        var errorDescription: String? {
            switch self {
            case .missingContext:
                return "驗證流程已過期，請重新登入"
            }
        }
    }

    @Published var email: String = ""
    @Published var password: String = ""
    @Published var isPasswordVisible: Bool = false
    @Published var isSubmitting: Bool = false
    @Published var banner: Banner?
    @Published var pendingOTP: PendingOTP?
    @Published var isVerifyingOTP: Bool = false

    private var bannerDismissTask: Task<Void, Never>?

    private var trimmedEmail: String {
        email.trimmingCharacters(in: .whitespacesAndNewlines)
    }

    private var trimmedPassword: String {
        password.trimmingCharacters(in: .whitespacesAndNewlines)
    }

    var isEmailValid: Bool {
        let regex = "[A-Z0-9a-z._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,64}"
        return NSPredicate(format: "SELF MATCHES %@", regex).evaluate(with: trimmedEmail)
    }

    var isPasswordValid: Bool {
        !trimmedPassword.isEmpty
    }

    @Published var emailValidated: Bool = false

    var shouldShowPasswordError: Bool {
        !password.isEmpty && !isPasswordValid
    }

    var emailErrorMessage: String? {
        (emailValidated && !isEmailValid) ? "請輸入有效的 Email 格式" : nil
    }

    var passwordErrorMessage: String? {
        shouldShowPasswordError ? "密碼不可為空" : nil
    }

    var canSubmit: Bool {
        isEmailValid && isPasswordValid && !isSubmitting
    }

    /// 驗證輸入並觸發登入流程。
    func submit() async {
        guard canSubmit else {
            if !isEmailValid {
                showBanner(title: "無效的 Email", message: "請確認電子郵件格式是否正確", kind: .error)
            } else if !isPasswordValid {
                showBanner(title: "請輸入密碼", message: "登入需要您的帳號密碼", kind: .error)
            }
            return
        }

        isSubmitting = true
        defer { isSubmitting = false }

        do {
            let profile = try await AuthManager.shared.signIn(email: trimmedEmail, password: trimmedPassword)
            showBanner(title: "登入成功", message: "歡迎回來，\(profile.displayName)", kind: .success)
            clearSensitiveFields()
        } catch let signInError as AuthManager.SignInError {
            switch signInError {
            case .otpRequired(let email, let password):
                pendingOTP = PendingOTP(email: email, password: password)
                showBanner(title: "需要 OTP 驗證", message: signInError.errorDescription ?? "請輸入 OTP 驗證碼", kind: .error)
                clearSensitiveFields()
            }
        } catch {
            showBanner(title: "登入失敗", message: resolveMessage(from: error), kind: .error)
        }
    }

    private func resolveMessage(from error: Error) -> String {
        if let apiError = error as? ApiError {
            return apiError.errorDescription ?? "未知錯誤"
        }
        return error.localizedDescription
    }

    /// 登入成功後清除密碼等敏感資料。
    private func clearSensitiveFields() {
        password = ""
    }

    /// 使用 OTP 完成登入流程。
    func completeOTP(otpCode: String?) async -> Result<Void, Error> {
        guard let pendingOTP else {
            return .failure(OTPFlowError.missingContext)
        }

        isVerifyingOTP = true
        defer { isVerifyingOTP = false }

        do {
            let profile = try await AuthManager.shared.signInWithOTP(
                email: pendingOTP.email,
                password: pendingOTP.password,
                otpCode: otpCode
            )
            self.pendingOTP = nil
            showBanner(title: "登入成功", message: "歡迎回來，\(profile.displayName)", kind: .success)
            clearSensitiveFields()
            return .success(())
        } catch {
            showBanner(title: "登入失敗", message: resolveMessage(from: error), kind: .error)
            return .failure(error)
        }
    }

    /// 顯示短暫提示讓使用者理解目前狀態。
    private func showBanner(title: String, message: String, kind: Banner.Kind) {
        bannerDismissTask?.cancel()
        banner = Banner(title: title, message: message, kind: kind)
        bannerDismissTask = Task { [weak self] in
            try? await Task.sleep(nanoseconds: 2_200_000_000)
            await MainActor.run { self?.banner = nil }
        }
    }
}
