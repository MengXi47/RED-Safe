//
//  network.swift
//  RED Safe
//
//  Created by boen on 2025/5/26.
//

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
    case internalServerError                = 500
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
    let user_id:        UUID?
    let user_name:      String?
    let serial_number: [String]?
}

/// 註冊請求回應模型
struct SignUpResponse: Codable {
    let error_code:     ErrorCode
    let user_id: UUID?
}

/// 網路錯誤自訂列舉
enum NetworkError: Error {
    case invalidURL                     // URL 無效
    case serverError(statusCode: Int)   // 伺服器錯誤
    case decodingError                  // JSON 資料解碼失敗
    case unknown(Error)                 // 其他未知錯誤
}

/// 單例模式的網路管理類別，負責發送各種 API 請求
class Network: NSObject {
    static let shared = Network()
    private override init() {}

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

        // 3. 編碼請求主體
        let body = SignInRequest(email: email, password: password)
        do {
            request.httpBody = try JSONEncoder().encode(body)
        } catch {
            completion(.failure(.unknown(error)))
            return
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
            do {
                let result = try JSONDecoder().decode(SignInResponse.self, from: data)
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
            // 印出伺服器回傳的原始 JSON 字串
            if let bodyString = String(data: data, encoding: .utf8) {
                print("📥 SignUp response body: \(bodyString)")
            }
            do {
                let result = try JSONDecoder().decode(SignUpResponse.self, from: data)
                completion(.success(result))
            } catch {
                completion(.failure(.decodingError))
            }
        }.resume()
    }
}
