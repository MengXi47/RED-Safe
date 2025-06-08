import Foundation

/// 伺服器回傳的錯誤代碼對應列舉
enum ErrorCode: Int, Codable {
    case success                            = 0
    case unknownEndpoint                    = 99
    case invalidJSON                        = 100
    case invalidSerialNumberFormat          = 101
    case invalidAPNSTokenFormat             = 102
    case invalidEmailFormat                 = 103
    case invalidUsernameFormat              = 104
    case invalidPasswordFormat              = 105
    case emailOrPasswordError               = 201
    case edgeDeviceAlreadyRegistered        = 301
    case emailAlreadyRegistered             = 302
    case bindingAlreadyExists               = 303
    case missingSerialNumberOrVersion       = 401
    case missingEmailOrUsernameOrPassword   = 402
    case missingEmailOrPassword             = 403
    case missingUserIdOrAPNSToken           = 404
    case missingUserIdOrSerialNumber        = 405
    case missingRefreshToken                = 406
    case missingAccessToken                 = 407
    case internalServerError                = 500
    case refreshTokenExpired                = 501
    case refreshTokenInvalid                = 502
    case accessTokenExpired                 = 503
    case accessTokenInvalid                 = 504
}

extension ErrorCode {
    /// 依錯誤碼回傳中文描述
    var localizedDescription: String {
        switch self {
        case .success:                        return "成功"
        case .emailAlreadyRegistered:         return "此 Email 已被註冊"
        case .edgeDeviceAlreadyRegistered:    return "裝置已註冊"
        case .bindingAlreadyExists:           return "綁定已存在"
        case .emailOrPasswordError:           return "Email 或 Password 錯誤"
        case .invalidEmailFormat:             return "Email 格式不正確"
        case .invalidPasswordFormat:          return "密碼格式不正確"
        case .internalServerError:            return "伺服器發生錯誤"
        case .missingRefreshToken:            return "缺少 Refresh Token"
        case .missingAccessToken:             return "缺少 Access Token"
        case .refreshTokenExpired:            return "Refresh Token 已過期"
        case .refreshTokenInvalid:            return "Refresh Token 無效"
        case .accessTokenExpired:             return "Access Token 已過期"
        case .accessTokenInvalid:             return "Access Token 無效"
        default:                              return "未知錯誤（\(rawValue)）"
        }
    }
}

/// 登入請求所需的資料
struct SignInRequest: Codable {
    let email:      String
    let password:   String
}

/// 註冊請求所需的資料
struct SignUpRequest: Codable {
    let email:      String
    let user_name:  String
    let password:   String
}

/// 登入請求回應模型
struct SignInResponse: Codable {
    let error_code:     ErrorCode
    let user_name:      String?
    let access_token:   String?
    var refresh_token:  String?
}

/// 註冊請求回應模型
struct SignUpResponse: Codable {
    let error_code:     ErrorCode
    let user_name:      String?
    let email:          String?
}

/// Refresh Token API 回應模型
struct RefreshResponse: Codable {
    let access_token: String?
    let error_code:   ErrorCode
}

/// 取得使用者資訊回應模型
struct UserInfoResponse: Codable {
    let error_code:    ErrorCode
    let user_name:     String?
    let email:         String?
    let serial_number: [String]?
}

/// 回應中包含 `error_code` 的通用協定
protocol HasErrorCode: Decodable {
    var error_code: ErrorCode { get }
}

extension SignInResponse: HasErrorCode {}
extension SignUpResponse: HasErrorCode {}
extension RefreshResponse: HasErrorCode {}
extension UserInfoResponse: HasErrorCode {}

/// 網路錯誤自訂列舉
enum NetworkError: Error {
    case invalidURL                     // URL 無效
    case serverError(statusCode: Int)   // 伺服器錯誤
    case decodingError                  // JSON 資料解碼失敗
    case unknown(Error)                 // 其他未知錯誤
}

/// 單例模式的網路管理類別，負責所有 API 請求的發送與重試機制
class Network: NSObject {
    static let shared = Network()
    private override init() {}
    
    /// 通用請求函式
    /// - Parameters:
    ///   - request: 已組裝好的 URLRequest
    ///   - retry: 是否在存取令牌過期時自動重試（預設 true）
    ///   - completion: 回傳解析後的結果或 NetworkError
    private func sendRequest<T: HasErrorCode>(_ request: URLRequest, retry: Bool = true, completion: @escaping (Result<T, NetworkError>) -> Void) {
        var request = request
        // 如有 Access Token，將其加入 Authorization 標頭
        if let token = AuthManager.shared.accessToken {
            request.setValue("Bearer \(token)", forHTTPHeaderField: "Authorization")
        }

        // 印出發送的請求資訊
        print("➡️ \(request.httpMethod ?? "") \(request.url?.absoluteString ?? "")")
        if let headers = request.allHTTPHeaderFields { print("Headers: \(headers)") }
        if let body = request.httpBody, let bodyStr = String(data: body, encoding: .utf8) {
            print("Body: \(bodyStr)")
        }
        
        URLSession.shared.dataTask(with: request) { data, response, error in
            // 檢查網路層級錯誤
            if let error = error {
                completion(.failure(.unknown(error)))
                return
            }
            // 驗證回傳是否為 HTTPURLResponse
            guard let http = response as? HTTPURLResponse else {
                completion(.failure(.serverError(statusCode: -1)))
                return
            }
            guard let data = data else {
                completion(.failure(.serverError(statusCode: http.statusCode)))
                return
            }

            // 印出回傳資訊
            print("⬅️ status: \(http.statusCode)")
            print("Headers: \(http.allHeaderFields)")
            if let bodyStr = String(data: data, encoding: .utf8) { print("Body: \(bodyStr)") }
            do {
                // 將回傳 JSON 反序列化為對應模型
                let result = try JSONDecoder().decode(T.self, from: data)
                // 若 Access Token 過期或無效，嘗試以 Refresh Token 自動續期並重試
                if retry && (result.error_code == .accessTokenExpired || result.error_code == .accessTokenInvalid) {
                    AuthManager.shared.refreshAccessToken { success in
                        if success {
                            self.sendRequest(request, retry: false, completion: completion)
                        } else {
                            completion(.success(result))
                        }
                    }
                    return
                }
                completion(.success(result))
            } catch {
                completion(.failure(.decodingError))
            }
        }.resume()
    }

    /// 登入請求
    /// - Parameters:
    ///   - email: 使用者 email
    ///   - password: 使用者密碼
    ///   - completion: 結果回呼，成功回傳 SignInResponse，失敗回傳 NetworkError
    func signIn(email: String, password: String, completion: @escaping (Result<SignInResponse, NetworkError>) -> Void) {
        // 1. 建立 URL
        guard let url = URL(string: "https://api.redsafe-tw.com/user/signin") else {
            completion(.failure(.invalidURL))
            return
        }

        // 2. 組裝 URLRequest
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")

        print("➡️ POST \(url.absoluteString)")
        print("Headers: \(request.allHTTPHeaderFields ?? [:])")

        // 3. 編碼請求主體
        let body = SignInRequest(email: email, password: password)
        do {
            request.httpBody = try JSONEncoder().encode(body)
        } catch {
            completion(.failure(.unknown(error)))
            return
        }
        if let bodyStr = String(data: request.httpBody ?? Data(), encoding: .utf8) {
            print("Body: \(bodyStr)")
        }

        // 4. 發送請求
        URLSession.shared.dataTask(with: request) { data, response, error in
            // 網路層錯誤
            if let error = error {
                completion(.failure(.unknown(error)))
                return
            }
            // 驗證 HTTP 狀態碼
            guard let http = response as? HTTPURLResponse else {
                completion(.failure(.serverError(statusCode: -1)))
                return
            }
            // Debug: 印出完整 HTTP 回應
            print("⬅️ status: \(http.statusCode)")
            print("Headers: \(http.allHeaderFields)")
            if let bodyData = data, let bodyString = String(data: bodyData, encoding: .utf8) {
                print("Body: \(bodyString)")
            }
            // 解析 Refresh Token
            var refreshToken: String?
            if let cookie = http.value(forHTTPHeaderField: "Set-Cookie"),
               let range  = cookie.range(of: "refresh_token=") {
                let sub = cookie[range.upperBound...]
                refreshToken = sub.split(separator: ";").first.map(String.init)
            }
            // 解析 JSON
            guard let data = data else {
                completion(.failure(.serverError(statusCode: http.statusCode)))
                return
            }
            do {
                var result = try JSONDecoder().decode(SignInResponse.self, from: data)
                // 若 JSON 未帶 refresh_token，改用 Cookie 解析到的值
                if result.refresh_token == nil {
                    result.refresh_token = refreshToken
                }
                print("Access Token: \(result.access_token ?? "nil")")
                print("Refresh Token: \(result.refresh_token ?? "nil")")
                completion(.success(result))
            } catch {
                completion(.failure(.decodingError))
            }
        }.resume()
    }
    
    /// 註冊請求
    /// - Parameters:
    ///   - email: 使用者 email
    ///   - userName: 使用者名稱
    ///   - password: 使用者密碼
    ///   - completion: 結果回呼，成功回傳 SignUpResponse，失敗回傳 NetworkError
    func signUp(email: String, userName: String, password: String, completion: @escaping (Result<SignUpResponse, NetworkError>) -> Void) {
        // 1. 建立 URL
        guard let url = URL(string: "https://api.redsafe-tw.com/user/signup") else {
            completion(.failure(.invalidURL))
            return
        }
        // 2. 組裝 URLRequest
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")
        // 3. 編碼請求主體
        let body = SignUpRequest(email: email, user_name: userName, password: password)
        do {
            request.httpBody = try JSONEncoder().encode(body)
        } catch {
            completion(.failure(.unknown(error)))
            return
        }
        if let bodyStr = String(data: request.httpBody ?? Data(), encoding: .utf8) {
            print("Body: \(bodyStr)")
        }
        // 4. 發送請求
        URLSession.shared.dataTask(with: request) { data, response, error in
            // 網路層錯誤
            if let error = error {
                completion(.failure(.unknown(error)))
                return
            }
            // 驗證 HTTP 狀態碼
            guard let http = response as? HTTPURLResponse else {
                completion(.failure(.serverError(statusCode: -1)))
                return
            }
            // 解析 JSON
            guard let data = data else {
                completion(.failure(.serverError(statusCode: http.statusCode)))
                return
            }
            print("⬅️ status: \(http.statusCode)")
            print("Headers: \(http.allHeaderFields)")
            if let bodyString = String(data: data, encoding: .utf8) {
                print("Body: \(bodyString)")
            }
            do {
                let result = try JSONDecoder().decode(SignUpResponse.self, from: data)
                completion(.success(result))
            } catch {
                completion(.failure(.decodingError))
            }
        }.resume()
    }
    /// Refresh Access Token
    func refreshAccessToken(refreshToken: String, completion: @escaping (Result<RefreshResponse, NetworkError>) -> Void) {
        guard let url = URL(string: "https://api.redsafe-tw.com/auth/refresh") else {
            completion(.failure(.invalidURL))
            return
        }
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        request.setValue("refresh_token=\(refreshToken)", forHTTPHeaderField: "Cookie")
        
        URLSession.shared.dataTask(with: request) { data, response, error in
            if let error = error {
                completion(.failure(.unknown(error)))
                return
            }
            guard let http = response as? HTTPURLResponse else {
                completion(.failure(.serverError(statusCode: -1)))
                return
            }
            guard let data = data else {
                completion(.failure(.serverError(statusCode: http.statusCode)))
                return
            }
            do {
                let result = try JSONDecoder().decode(RefreshResponse.self, from: data)
                completion(.success(result))
            } catch {
                completion(.failure(.decodingError))
            }
        }.resume()
    }

    /// 取得使用者資訊
    func getUserInfo(completion: @escaping (Result<UserInfoResponse, NetworkError>) -> Void) {
        guard let url = URL(string: "https://api.redsafe-tw.com/user/all") else {
            completion(.failure(.invalidURL))
            return
        }

        var request = URLRequest(url: url)
        request.httpMethod = "GET"

        sendRequest(request) { (result: Result<UserInfoResponse, NetworkError>) in
            completion(result)
        }
    }
}
