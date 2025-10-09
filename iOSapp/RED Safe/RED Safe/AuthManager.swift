import Foundation
import Security

// MARK: - Keychain Utility

/// KeychainHelper 專職封裝敏感資訊的讀寫，遵循單一職責原則確保安全性元件集中管理。
enum KeychainHelper {
    @discardableResult
    static func save(key: String, data: Data) -> OSStatus {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrAccount as String: key,
            kSecValueData as String: data
        ]
        SecItemDelete(query as CFDictionary)
        return SecItemAdd(query as CFDictionary, nil)
    }

    static func load(key: String) -> Data? {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrAccount as String: key,
            kSecReturnData as String: kCFBooleanTrue as Any,
            kSecMatchLimit as String: kSecMatchLimitOne
        ]
        var result: AnyObject?
        let status = SecItemCopyMatching(query as CFDictionary, &result)
        if status == errSecSuccess {
            return result as? Data
        }
        return nil
    }

    static func loadString(key: String) -> String? {
        guard let data = load(key: key) else { return nil }
        return String(data: data, encoding: .utf8)
    }

    static func saveString(_ value: String, for key: String) {
        save(key: key, data: Data(value.utf8))
    }

    static func delete(key: String) {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrAccount as String: key
        ]
        SecItemDelete(query as CFDictionary)
    }
}

// MARK: - Authentication Store

/// AuthManager 作為認證領域的唯一協調者，負責狀態生命週期、Token 管理與使用者資訊同步 (符合 SRP/OCP)。
@MainActor
final class AuthManager: ObservableObject {
    enum Phase: Equatable {
        case launching
        case refreshing
        case authenticated
        case signedOut
    }

    enum SignInError: LocalizedError {
        case otpRequired(email: String, password: String)

        var errorDescription: String? {
            switch self {
            case .otpRequired:
                return "此帳號已啟用二階段驗證，請輸入 OTP 驗證碼"
            }
        }
    }

    struct UserProfile: Equatable {
        var email: String
        var displayName: String

        func renamed(_ name: String) -> UserProfile {
            var copy = self
            copy.displayName = name
            return copy
        }
    }

    static let shared = AuthManager()

    @Published private(set) var phase: Phase = .launching
    @Published private(set) var profile: UserProfile?
    @Published private(set) var isWorking: Bool = false
    @Published private(set) var lastErrorDescription: String?

    var accessToken: String? { tokens?.accessToken }
    var isLoggedIn: Bool { phase == .authenticated }
    var userName: String? { profile?.displayName }

    private struct SessionTokens {
        var accessToken: String
        var refreshToken: String
    }

    private var tokens: SessionTokens?

    private let refreshKey = "refreshtoken"
    private let usernameKey = "username"
    private let emailKey = "email"

    private init() {}

    // MARK: - Lifecycle

    /// 初始化時觸發自動登入流程，確保 App 啟動體驗一致。
    func bootstrap() {
        guard phase == .launching else { return }
        Task { await restoreSessionIfPossible() }
    }

    @discardableResult
    /// 嘗試透過 Refresh Token 還原會話；若失敗則回復到登出狀態。
    func restoreSessionIfPossible() async -> Bool {
        phase = .refreshing
        guard let refresh = storedRefreshToken else {
            tokens = nil
            profile = nil
            phase = .signedOut
            return false
        }

        do {
            let response = try await APIClient.shared.refreshAccessToken(refreshToken: refresh)
            let access = response.accessToken
            guard !access.isEmpty else {
                clearPersistedSession()
                phase = .signedOut
                return false
            }

            tokens = SessionTokens(accessToken: access, refreshToken: refresh)
            hydrateProfileIfNeeded()
            phase = .authenticated
            return true
        } catch {
            clearPersistedSession()
            phase = .signedOut
            lastErrorDescription = error.localizedDescription
            return false
        }
    }

    // MARK: - Auth Flows

    @discardableResult
    /// 驗證帳密並建立全新會話，成功後回傳使用者檔案。
    func signIn(email: String, password: String) async throws -> UserProfile {
        isWorking = true
        defer { isWorking = false }

        let response = try await APIClient.shared.signIn(email: email, password: password)
        if response.requiresOTP {
            throw SignInError.otpRequired(email: email, password: password)
        }

        return try finalizeSignIn(with: response, email: email)
    }

    @discardableResult
    func signInWithOTP(email: String, password: String, otpCode: String?, backupCode: String?) async throws -> UserProfile {
        isWorking = true
        defer { isWorking = false }

        let response = try await APIClient.shared.signInWithOTP(email: email, password: password, otpCode: otpCode, backupCode: backupCode)
        if response.requiresOTP {
            throw ApiError.invalidPayload(reason: "伺服器回傳要求再次輸入 OTP，請稍後再試")
        }
        return try finalizeSignIn(with: response, email: email)
    }

    @discardableResult
    /// 透過 Refresh Token 續期 Access Token，維持登入狀態。
    func refreshAccessToken(refreshToken: String? = nil) async -> Bool {
        let refresh = refreshToken ?? tokens?.refreshToken ?? storedRefreshToken
        guard let refresh, !refresh.isEmpty else {
            tokens = nil
            profile = nil
            phase = .signedOut
            return false
        }

        do {
            let response = try await APIClient.shared.refreshAccessToken(refreshToken: refresh)
            let access = response.accessToken
            guard !access.isEmpty else {
                clearPersistedSession()
                phase = .signedOut
                return false
            }

            if tokens != nil {
                tokens?.accessToken = access
                tokens?.refreshToken = refresh
            } else {
                tokens = SessionTokens(accessToken: access, refreshToken: refresh)
            }

            hydrateProfileIfNeeded()
            phase = .authenticated
            return true
        } catch {
            clearPersistedSession()
            phase = .signedOut
            lastErrorDescription = error.localizedDescription
            return false
        }
    }

    /// 手動登出並清除本地憑證。
    func signOut() {
        tokens = nil
        profile = nil
        clearPersistedSession()
        phase = .signedOut
    }

    private func finalizeSignIn(with response: SignInResponse, email: String) throws -> UserProfile {
#if DEBUG
        print("🔐 SignInResponse debug userName=\(response.userName ?? "nil") access=\(response.accessToken ?? "<nil>") refresh=\(response.refreshToken ?? "<nil>")")
#endif
        guard let access = response.accessToken, !access.isEmpty else {
            throw ApiError.invalidPayload(reason: "伺服器未回傳 access token")
        }
        guard let refresh = response.refreshToken, !refresh.isEmpty else {
            throw ApiError.invalidPayload(reason: "伺服器未回傳 refresh token")
        }

        let resolvedName = response.normalizedUserName ?? email
        let profile = UserProfile(email: email, displayName: resolvedName)

        persistSession(refreshToken: refresh, displayName: resolvedName, email: email)
        tokens = SessionTokens(accessToken: access, refreshToken: refresh)
        self.profile = profile
        phase = .authenticated
        return profile
    }

    @discardableResult
    /// 更新顯示名稱並同步 Keychain 中的快取。
    func updateUserName(to newName: String) async throws -> ApiErrorCode {
        let result = try await APIClient.shared.updateUserName(newName)
        KeychainHelper.saveString(newName, for: usernameKey)
        if let current = profile {
            profile = current.renamed(newName)
        }
        return result
    }

    // MARK: - Helpers

    private var storedRefreshToken: String? {
        KeychainHelper.loadString(key: refreshKey)
    }

    private func hydrateProfileIfNeeded() {
        guard profile == nil else { return }
        let displayName = KeychainHelper.loadString(key: usernameKey) ?? "使用者"
        let email = KeychainHelper.loadString(key: emailKey) ?? ""
        profile = UserProfile(email: email, displayName: displayName)
    }

    private func persistSession(refreshToken: String, displayName: String, email: String) {
        KeychainHelper.saveString(refreshToken, for: refreshKey)
        KeychainHelper.saveString(displayName, for: usernameKey)
        KeychainHelper.saveString(email, for: emailKey)
    }

    private func clearPersistedSession() {
        KeychainHelper.delete(key: refreshKey)
        KeychainHelper.delete(key: usernameKey)
        KeychainHelper.delete(key: emailKey)
    }
}
