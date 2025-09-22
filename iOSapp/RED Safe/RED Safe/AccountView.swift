import SwiftUI

// MARK: - 帳號分頁

enum ProfileSheet: Identifiable {
    case displayName
    case password
    case push

    var id: String {
        switch self {
        case .displayName: return "display-name"
        case .password: return "password"
        case .push: return "push"
        }
    }
}

/// AccountView 管理使用者帳號資訊與裝置註冊。
struct AccountView: View {
    @ObservedObject var auth: AuthManager
    @ObservedObject var profileVM: ProfileViewModel
    @Binding var profileSheet: ProfileSheet?

    var body: some View {
        NavigationStack {
            ScrollView(showsIndicators: false) {
                VStack(spacing: 24) {
                    header
                    infoCard
                    pushCard
                    signOutButton
                }
                .padding(.horizontal, 20)
                .padding(.top, 32)
                .padding(.bottom, 48)
            }
            .background(Color(.systemGroupedBackground).ignoresSafeArea())
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
                case .push:
                    RegisterDeviceSheet { deviceId, token, deviceName, completion in
                        Task { @MainActor in
                            let success = await profileVM.registerDevice(deviceId: deviceId, apnsToken: token, deviceName: deviceName)
                            if success { profileSheet = nil }
                            completion(success)
                        }
                    }
                }
            }
            .navigationTitle("帳號")
            .navigationBarTitleDisplayMode(.inline)
        }
    }

    private var header: some View {
        VStack(alignment: .leading, spacing: 10) {
            Text("帳號與安全")
                .font(.system(size: 28, weight: .bold, design: .rounded))
                .foregroundColor(.primary)
            Text("管理個人資料與通知設定")
                .font(.subheadline)
                .foregroundColor(.secondary)
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
            }
        }
        .padding(20)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(RoundedRectangle(cornerRadius: 28, style: .continuous).fill(Color(uiColor: .secondarySystemGroupedBackground)))
        .overlay(RoundedRectangle(cornerRadius: 28, style: .continuous).stroke(Color.black.opacity(0.04)))
    }

    private var pushCard: some View {
        VStack(alignment: .leading, spacing: 16) {
            HStack {
                VStack(alignment: .leading, spacing: 6) {
                    Text("推播裝置")
                        .font(.headline)
                    if let device = profileVM.lastRegisteredDevice {
                        Text(device.deviceName ?? "未命名裝置")
                            .font(.subheadline)
                        Text(device.iosDeviceId)
                            .font(.caption.monospaced())
                            .foregroundColor(.secondary)
                        Text(device.apnsToken)
                            .font(.caption2.monospaced())
                            .foregroundColor(.secondary)
                            .lineLimit(2)
                    } else {
                        Text("尚未註冊任何 APNS 裝置")
                            .font(.subheadline)
                            .foregroundColor(.secondary)
                    }
                }
                Spacer()
                RoundedButton(symbol: "antenna.radiowaves.left.and.right") {
                    profileSheet = .push
                }
            }
        }
        .padding(20)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(RoundedRectangle(cornerRadius: 28, style: .continuous).fill(Color(uiColor: .secondarySystemGroupedBackground)))
        .overlay(RoundedRectangle(cornerRadius: 28, style: .continuous).stroke(Color.black.opacity(0.04)))
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

private struct RoundedButton: View {
    let symbol: String
    let action: () -> Void

    var body: some View {
        Button(action: action) {
            Image(systemName: symbol)
                .font(.headline)
                .foregroundStyle(.white)
                .frame(width: 46, height: 46)
                .background(Circle().fill(Color.white.opacity(0.22)))
        }
        .buttonStyle(.plain)
        .shadow(color: Color.black.opacity(0.18), radius: 12, x: 0, y: 8)
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

private struct RegisterDeviceSheet: View {
    @Environment(\.dismiss) private var dismiss
    @State private var deviceId = ""
    @State private var apnsToken = ""
    @State private var deviceName = ""
    @State private var isSubmitting = false

    let onSubmit: (String?, String, String?, @escaping (Bool) -> Void) -> Void

    private var normalizedDeviceId: String? {
        let trimmed = deviceId.trimmed
        return trimmed.isEmpty ? nil : trimmed
    }
    private var normalizedDeviceName: String? {
        let trimmed = deviceName.trimmed
        return trimmed.isEmpty ? nil : trimmed
    }
    private var normalizedToken: String { apnsToken.trimmed }
    private var isDeviceIdValid: Bool {
        guard let value = normalizedDeviceId else { return true }
        return UUID(uuidString: value) != nil
    }
    private var isFormValid: Bool { !normalizedToken.isEmpty && isDeviceIdValid }

    var body: some View {
        NavigationStack {
            Form {
                Section("裝置資訊") {
                    TextField("iOS Device ID (UUID)", text: $deviceId)
                        .textInputAutocapitalization(.never)
                        .autocorrectionDisabled()
                    TextField("裝置名稱 (選填)", text: $deviceName)
                }
                Section("APNS Token") {
                    TextEditor(text: $apnsToken)
                        .frame(height: 120)
                        .font(.footnote.monospaced())
                }
                if !isDeviceIdValid {
                    Text("Device ID 必須為合法的 UUID")
                        .font(.footnote)
                        .foregroundColor(.red)
                }
                if normalizedToken.isEmpty {
                    Text("APNS Token 不可為空")
                        .font(.footnote)
                        .foregroundColor(.red)
                }
            }
            .navigationTitle("推播裝置註冊")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("取消") { dismiss() }
                        .disabled(isSubmitting)
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("送出") {
                        isSubmitting = true
                        onSubmit(normalizedDeviceId, normalizedToken, normalizedDeviceName) { success in
                            isSubmitting = false
                            if success { dismiss() }
                        }
                    }
                    .disabled(!isFormValid || isSubmitting)
                }
            }
        }
    }
}
