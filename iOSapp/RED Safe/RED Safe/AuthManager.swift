import Foundation
import Security

class KeychainHelper {
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
        var dataTypeRef: AnyObject?
        let status = SecItemCopyMatching(query as CFDictionary, &dataTypeRef)
        if status == errSecSuccess {
            return dataTypeRef as? Data
        }
        return nil
    }
    
    static func delete(key: String) {
        let query: [String: Any] = [
            kSecClass as String: kSecClassGenericPassword,
            kSecAttrAccount as String: key
        ]
        SecItemDelete(query as CFDictionary)
    }
}

/// 單例模式管理身分驗證流程，包含登入、續期與登出
final class AuthManager: ObservableObject {
    /// 全域唯一實例
    static let shared = AuthManager()
    private init() {}

    /// Keychain 中儲存 Refresh Token 的鍵名
    private let refreshKey = "refresh_token"
    let nameKey = "user_name"

    /// 追蹤登入狀態，供 UI 綁定
    @Published private(set) var isLoggedIn: Bool = false
    /// 載入狀態，供 Loading 視圖綁定
    @Published var isLoading: Bool = false
    /// 暫存的 Access Token，僅存於記憶體
    private(set) var accessToken: String?
    /// 登入使用者名稱
    private(set) var userName: String?

    /// 嘗試從 Keychain 載入 Refresh Token，並續期取得新的 Access Token
    func loadSavedSession(completion: ((Bool) -> Void)? = nil) {
        // 從 UserDefaults 讀取儲存的 user_name
        self.userName = UserDefaults.standard.string(forKey: nameKey)

        guard let tokenData = KeychainHelper.load(key: refreshKey),
              let refresh = String(data: tokenData, encoding: .utf8) else {
            completion?(false)
            return
        }
        refreshAccessToken(refreshToken: refresh, completion: completion)
    }

    /// 執行登入，成功後儲存 Refresh Token 並更新狀態
    /// - Parameters:
    ///   - email: 使用者 Email
    ///   - password: 使用者 密碼
    ///   - completion: 回傳 SignInResponse 或 NetworkError
    func signIn(email: String, password: String, completion: @escaping (Result<SignInResponse, NetworkError>) -> Void) {
        // 執行登入，不切換全域 Loading
        Network.shared.signIn(email: email, password: password) { result in
            DispatchQueue.main.async {
                switch result {
                case .success(let response):
                    if response.error_code == .success,
                       let access = response.access_token,
                       let refresh = response.refresh_token {
                        self.accessToken = access
                        self.userName = response.user_name
                        if let name = response.user_name {
                            UserDefaults.standard.set(name, forKey: self.nameKey)
                        }
                        KeychainHelper.save(key: self.refreshKey, data: Data(refresh.utf8))
                        self.isLoggedIn = true
                    }
                    completion(.success(response))
                case .failure(let error):
                    completion(.failure(error))
                }
            }
        }
    }

    /// 使用 Refresh Token 續期 Access Token
    /// - Parameters:
    ///   - refreshToken: 可選的外部提供 Token，預設會從 Keychain 讀取
    ///   - completion: 回傳是否續期成功
    func refreshAccessToken(refreshToken: String? = nil, completion: ((Bool) -> Void)? = nil) {
        // 開始 Token 續期，啟動 Loading
        self.isLoading = true
        print("🔄 refreshAccessToken 開始，isLoading = \(self.isLoading)")
            // 從參數或 Keychain 嘗試取得 Refresh Token
        guard let token = refreshToken ?? KeychainHelper.load(key: refreshKey).flatMap({ String(data: $0, encoding: .utf8) }) else {
                // 續期失敗，停止 Loading
                self.isLoading = false
            self.isLoggedIn = false
            completion?(false)
            return
        }
        print("🔑 使用的 refreshToken: \(token)")
        print("📡 呼叫 Network.shared.refreshAccessToken")
            // 向後端發送 refresh token 續期請求
        Network.shared.refreshAccessToken(refreshToken: token) { result in
                // 切回主執行緒更新狀態
            DispatchQueue.main.async {
                print("✅ 切回主線程，收到續期結果: \(result)")
                    // 處理成功回應，更新 Access Token
                switch result {
                case .success(let response):
                    print("🎉 續期成功，error_code = \(response.error_code), access_token = \(response.access_token ?? "nil")")
                    if response.error_code == .success, let token = response.access_token {
                        self.accessToken = token
                        self.isLoggedIn = true
                        // 取得使用者資訊
                        Network.shared.getUserInfo { userResult in
                            if case .success(let userInfo) = userResult, userInfo.error_code == .success {
                                self.userName = userInfo.user_name
                                if let name = userInfo.user_name {
                                    UserDefaults.standard.set(name, forKey: self.nameKey)
                                }
                            }
                        }
                        completion?(true)
                    } else {
                        self.isLoggedIn = false
                        completion?(false)
                    }
                    // 處理失敗情況，標記為未登入
                case .failure:
                    print("❌ 續期請求失敗")
                    self.isLoggedIn = false
                    completion?(false)
                }
                // Token 續期結束，停止 Loading
                self.isLoading = false
            }
        }
    }

    /// 登出並清除所有 Token 與使用者資料
    func signOut() {
        accessToken = nil
        userName = nil
        KeychainHelper.delete(key: refreshKey)
        UserDefaults.standard.removeObject(forKey: nameKey)
        isLoggedIn = false
    }

    /// 僅清除暫存的 Access Token
    func clearAccessToken() {
        accessToken = nil
    }
}
