import SwiftUI
import UIKit

/// SignUpView 負責引導使用者建立帳號與回饋驗證訊息。
struct SignUpView: View {
    @Environment(\.dismiss) private var dismiss
    @StateObject private var viewModel = SignUpViewModel()
    @FocusState private var focusedField: SignUpViewModel.Field?
    @State private var animateBackground = false

    var body: some View {
        ZStack {
            backgroundLayer
            ScrollView(showsIndicators: false) {
                VStack(spacing: 32) {
                    heroSection
                    formSection
                    tipsSection
                }
                .padding(.horizontal, 24)
                .padding(.top, 32)
                .padding(.bottom, 48)
            }
        }
        .contentShape(Rectangle())
        .onTapGesture { focusedField = nil }
        .onAppear { animateBackground = true }
        .onChange(of: viewModel.banner) { banner in
            guard banner != nil else { return }
            UIImpactFeedbackGenerator(style: .soft).impactOccurred()
        }
        .onChange(of: viewModel.shouldDismiss) { shouldDismiss in
            if shouldDismiss {
                dismiss()
            }
        }
        .onChange(of: focusedField) { oldValue, newValue in
            if oldValue == .email, newValue != .email {
                viewModel.emailValidated = true
            }
        }
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
                        .scaleEffect(1.1)
                }
            }
        }
        .animation(.spring(response: 0.45, dampingFraction: 0.85), value: viewModel.banner)
        .toolbar { backButton }
        .navigationBarBackButtonHidden(true)
        .safeAreaInset(edge: .top) {
            Color.clear.frame(height: 4)
        }
    }

    private var backButton: some ToolbarContent {
        ToolbarItem(placement: .cancellationAction) {
            Button {
                dismiss()
            } label: {
                Label("Back", systemImage: "chevron.backward")
            }
            .tint(.white)
        }
    }

    private var backgroundLayer: some View {
        ZStack {
            LinearGradient(
                colors: [
                    Color(red: 230/255, green: 245/255, blue: 255/255),
                    Color(red: 200/255, green: 230/255, blue: 255/255),
                    Color(red: 255/255, green: 255/255, blue: 255/255)
                ],
                startPoint: .topLeading,
                endPoint: .bottomTrailing
            )
            .ignoresSafeArea()

            Circle()
                .fill(Color.white.opacity(0.16))
                .frame(width: animateBackground ? 360 : 220)
                .blur(radius: 40)
                .offset(x: -150, y: animateBackground ? -250 : -140)
                .animation(.easeOut(duration: 1.0), value: animateBackground)

            Circle()
                .fill(Color.white.opacity(0.12))
                .frame(width: animateBackground ? 320 : 200)
                .blur(radius: 34)
                .offset(x: 160, y: animateBackground ? 260 : 150)
                .animation(.easeOut(duration: 1.0).delay(0.05), value: animateBackground)
        }
    }

    private var heroSection: some View {
        VStack(spacing: 14) {
            Image("RED_Safe_icon1")
                .resizable()
                .scaledToFit()
                .frame(width: 150, height: 150)
                .shadow(color: Color.black.opacity(0.24), radius: 18, x: 0, y: 14)
                .scaleEffect(animateBackground ? 1 : 0.85)
                .opacity(animateBackground ? 1 : 0)
                .animation(.spring(response: 0.7, dampingFraction: 0.65).delay(0.05), value: animateBackground)

            VStack(spacing: 6) {
                Text("建立全新帳號")
                    .font(.system(size: 30, weight: .bold, design: .rounded))
                    .foregroundColor(.primary)
                Text("守護家人，只需 1 分鐘完成註冊")
                    .font(.callout)
                    .foregroundColor(.secondary)
            }
        }
    }

    private var formSection: some View {
        VStack(spacing: 22) {
            VStack(spacing: 16) {
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
                .onSubmit { focusedField = .userName }

                CustomTextField(
                    title: "顯示名稱 (1-16 字)",
                    text: $viewModel.userName,
                    icon: "person.crop.circle.fill",
                    keyboard: .default,
                    isSecure: false,
                    isValid: viewModel.shouldShowNameError ? viewModel.isNameValid : true,
                    errorText: viewModel.nameErrorMessage
                )
                .focused($focusedField, equals: .userName)
                .submitLabel(.next)
                .onSubmit { focusedField = .password }

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
                .submitLabel(.next)
                .onSubmit { focusedField = .confirmPassword }

                CustomTextField(
                    title: "確認密碼",
                    text: $viewModel.confirmPassword,
                    icon: "checkmark.lock.fill",
                    keyboard: .default,
                    isSecure: !viewModel.isConfirmVisible,
                    isValid: viewModel.shouldShowConfirmError ? viewModel.isConfirmValid : true,
                    errorText: viewModel.confirmErrorMessage,
                    trailingIcon: viewModel.isConfirmVisible ? "eye.slash" : "eye",
                    trailingAction: { viewModel.isConfirmVisible.toggle() }
                )
                .focused($focusedField, equals: .confirmPassword)
                .submitLabel(.done)
                .onSubmit { Task { await viewModel.submit() } }
            }

            Button(action: { Task { await viewModel.submit() } }) {
                ZStack {
                    RoundedRectangle(cornerRadius: 20, style: .continuous)
                        .fill(
                            LinearGradient(
                                colors: viewModel.canSubmit ? [
                                    Color(red: 120/255, green: 230/255, blue: 160/255),
                                    Color(red: 46/255, green: 204/255, blue: 113/255)
                                ] : [Color.white.opacity(0.22), Color.white.opacity(0.14)],
                                startPoint: .topLeading,
                                endPoint: .bottomTrailing
                            )
                        )
                    RoundedRectangle(cornerRadius: 20, style: .continuous)
                        .stroke(Color.white.opacity(viewModel.canSubmit ? 0.35 : 0.2))
                    HStack(spacing: 12) {
                        if viewModel.isSubmitting {
                            ProgressView()
                                .progressViewStyle(.circular)
                                .tint(.white)
                        } else {
                            Image(systemName: "sparkles")
                                .font(.title3)
                                .foregroundStyle(.white)
                        }
                        Text(viewModel.isSubmitting ? "註冊中" : "建立帳號")
                            .font(.headline.weight(.semibold))
                            .foregroundStyle(.white)
                    }
                    .padding(.vertical, 16)
                }
                .frame(height: 60)
            }
            .buttonStyle(.plain)
            .disabled(!viewModel.canSubmit)
            .shadow(color: Color.black.opacity(viewModel.canSubmit ? 0.3 : 0), radius: 24, x: 0, y: 18)
        }
        .padding(24)
        .background(
            RoundedRectangle(cornerRadius: 32, style: .continuous)
                .fill(.ultraThinMaterial)
                .overlay(
                    LinearGradient(
                        colors: [Color.white.opacity(0.22), Color.clear],
                        startPoint: .topLeading,
                        endPoint: .bottomTrailing
                    )
                    .clipShape(RoundedRectangle(cornerRadius: 32, style: .continuous))
                )
        )
        .overlay(
            RoundedRectangle(cornerRadius: 32, style: .continuous)
                .stroke(Color.white.opacity(0.16))
        )
        .shadow(color: Color.black.opacity(0.2), radius: 26, x: 0, y: 20)
    }

    private var tipsSection: some View {
        VStack(alignment: .leading, spacing: 16) {
            Text("註冊小提示")
                .font(.title3.weight(.semibold))
                .foregroundColor(.primary)
            VStack(alignment: .leading, spacing: 12) {
                TipRow(icon: "number.circle.fill", title: "16 字內顯示名稱", detail: "維持簡潔，方便家人成員辨識")
                TipRow(icon: "lock.square.stack", title: "安全密碼建議", detail: "至少 8 碼並混合英文字母與數字")
                TipRow(icon: "arrow.clockwise", title: "立即登入", detail: "註冊成功後將自動為您登入")
            }
            .padding(24)
            .glassCard(cornerRadius: 28)
        }
    }
}

private struct TipRow: View {
    let icon: String
    let title: String
    let detail: String

    var body: some View {
        HStack(alignment: .firstTextBaseline, spacing: 16) {
            Image(systemName: icon)
                .font(.title3)
                .foregroundColor(.accentColor)
                .frame(width: 30)
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
    let banner: SignUpViewModel.Banner

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

/// SignUpViewModel 處理註冊流程與表單驗證，確保業務邏輯與 UI 解耦。
@MainActor
final class SignUpViewModel: ObservableObject {
    enum Field: Hashable {
        case email
        case userName
        case password
        case confirmPassword
    }

    struct Banner: Identifiable, Equatable {
        enum Kind { case success, error }
        let id = UUID()
        let title: String
        let message: String
        let kind: Kind
    }

    @Published var email: String = ""
    @Published var emailValidated: Bool = false
    @Published var userName: String = ""
    @Published var password: String = ""
    @Published var confirmPassword: String = ""
    @Published var isPasswordVisible: Bool = false
    @Published var isConfirmVisible: Bool = false
    @Published var isSubmitting: Bool = false
    @Published var banner: Banner?
    @Published var shouldDismiss: Bool = false

    private var bannerTask: Task<Void, Never>?

    private var trimmedEmail: String { email.trimmingCharacters(in: .whitespacesAndNewlines) }
    private var trimmedUserName: String { userName.trimmingCharacters(in: .whitespacesAndNewlines) }
    private var trimmedPassword: String { password.trimmingCharacters(in: .whitespacesAndNewlines) }
    private var trimmedConfirm: String { confirmPassword.trimmingCharacters(in: .whitespacesAndNewlines) }

    var isEmailValid: Bool {
        let regex = "[A-Z0-9a-z._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,64}"
        return NSPredicate(format: "SELF MATCHES %@", regex).evaluate(with: trimmedEmail)
    }

    var isNameValid: Bool {
        !trimmedUserName.isEmpty && trimmedUserName.count <= 16
    }

    var isPasswordValid: Bool {
        let regex = "^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d!@#$%^&*()_+=-]{8,}$"
        return NSPredicate(format: "SELF MATCHES %@", regex).evaluate(with: trimmedPassword)
    }

    var isConfirmValid: Bool {
        !trimmedConfirm.isEmpty && trimmedPassword == trimmedConfirm
    }

    var shouldShowEmailError: Bool { !email.isEmpty && !isEmailValid }
    var shouldShowNameError: Bool { !userName.isEmpty && !isNameValid }
    var shouldShowPasswordError: Bool { !password.isEmpty && !isPasswordValid }
    var shouldShowConfirmError: Bool { !confirmPassword.isEmpty && !isConfirmValid }

    var emailErrorMessage: String? { (emailValidated && !isEmailValid) ? "請輸入有效的 Email 格式" : nil }
    var nameErrorMessage: String? { shouldShowNameError ? "顯示名稱需為 1-16 字元" : nil }
    var passwordErrorMessage: String? {
        shouldShowPasswordError ? "至少 8 碼並包含英文字與數字" : nil
    }
    var confirmErrorMessage: String? {
        shouldShowConfirmError ? "兩次密碼需一致" : nil
    }

    var canSubmit: Bool {
        isEmailValid && isNameValid && isPasswordValid && isConfirmValid && !isSubmitting
    }

    /// 驗證輸入並送出註冊請求，成功後自動登入。
    func submit() async {
        guard canSubmit else {
            if !isEmailValid {
                showBanner(title: "Email 格式有誤", message: "請確認輸入的電子郵件", kind: .error)
            } else if !isNameValid {
                showBanner(title: "顯示名稱不符合規範", message: "需為 1-16 字且不得為空", kind: .error)
            } else if !isPasswordValid {
                showBanner(title: "密碼強度不足", message: "建議混合大小寫與數字，至少 8 碼", kind: .error)
            } else if !isConfirmValid {
                showBanner(title: "密碼不一致", message: "請再次確認兩次密碼輸入相同", kind: .error)
            }
            return
        }

        isSubmitting = true
        defer { isSubmitting = false }

        do {
            _ = try await APIClient.shared.signUp(email: trimmedEmail, userName: trimmedUserName, password: trimmedPassword)
            showBanner(title: "註冊成功", message: "歡迎加入 RED Safe！", kind: .success)
            try? await AuthManager.shared.signIn(email: trimmedEmail, password: trimmedPassword)
            Task { [weak self] in
                try? await Task.sleep(nanoseconds: 1_100_000_000)
                await MainActor.run { self?.shouldDismiss = true }
            }
        } catch {
            showBanner(title: "註冊失敗", message: resolveMessage(from: error), kind: .error)
        }
    }

    private func resolveMessage(from error: Error) -> String {
        if let apiError = error as? ApiError {
            return apiError.errorDescription ?? "未知錯誤"
        }
        return error.localizedDescription
    }

    /// 顯示視覺化提示告知使用者操作結果。
    private func showBanner(title: String, message: String, kind: Banner.Kind) {
        bannerTask?.cancel()
        banner = Banner(title: title, message: message, kind: kind)
        bannerTask = Task { [weak self] in
            try? await Task.sleep(nanoseconds: 2_400_000_000)
            await MainActor.run { self?.banner = nil }
        }
    }
}
