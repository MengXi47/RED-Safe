//
//  network.swift
//  RED Safe
//
//  Created by boen on 2025/5/26.
//

// network.swift：處理與 RED Safe API Server 的網路請求

import Foundation
import UIKit

// MARK: - Request 載體

/// 登入請求所需的資料結構，會轉成 JSON 發送給伺服器
struct SignInRequest: Codable {
    let email: String
    let password: String
}

// MARK: - Response 模型

/// 登入請求回應模型，包含伺服器返回的 token 與其他使用者資訊
struct SignInResponse: Codable {
    let token: String
    // add other fields if needed
}

// MARK: - 網路錯誤定義

/// 定義網路請求可能發生的各種錯誤類型
enum NetworkError: Error {
    case invalidURL          // URL 無效
    case serverError(statusCode: Int)  // 伺服器回傳非 2xx 狀態碼
    case decodingError       // JSON 資料解碼失敗
    case unknown(Error)      // 其他未知錯誤
}

// MARK: - 網路管理

/// 單例模式的網路管理類別，負責發送各種 API 請求
class Network: NSObject, URLSessionDelegate {
    static let shared = Network()
    private override init() {}

    /// 發送使用者登入請求
    /// - Parameters:
    ///   - email: 使用者 email
    ///   - password: 使用者密碼
    ///   - completion: 完成後回傳 SignInResponse 或 NetworkError
    func signIn(email: String, password: String, completion: @escaping (Result<SignInResponse, NetworkError>) -> Void) {
        // 1. 建立 URL
        guard let url = URL(string: "https://113.61.152.89:30678/user/signin") else {
            completion(.failure(.invalidURL))
            return
        }
        print("🔍 發送 SignIn 請求至：\(url.absoluteString)")

        // 2. 設定 HTTP 請求方法與標頭
        var request = URLRequest(url: url)
        request.httpMethod = "POST"
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")

        // 3. 編碼請求主體為 JSON
        let body = SignInRequest(email: email, password: password)
        do {
            request.httpBody = try JSONEncoder().encode(body)
            if let json = String(data: request.httpBody!, encoding: .utf8) {
                print("📤 Request Body JSON: \(json)")
            }
        } catch {
            completion(.failure(.unknown(error)))
            return
        }

        // 4. 發送網路請求
        let session = URLSession(configuration: .default, delegate: self, delegateQueue: nil)
        session.dataTask(with: request) { data, response, error in
            print("📡 收到回應，開始處理...")
            if let error = error {
                print("❌ 網路錯誤：\(error.localizedDescription)")
                completion(.failure(.unknown(error)))
                return
            }
            guard let http = response as? HTTPURLResponse else {
                completion(.failure(.serverError(statusCode: -1)))
                return
            }
            // 輸出原始回傳 JSON（包含錯誤訊息）
            if let data = data, let responseString = String(data: data, encoding: .utf8) {
                print("📥 Response JSON: \(responseString)")
                DispatchQueue.main.async {
                    // 顯示系統 Alert 彈窗
                    let alert = UIAlertController(
                        title: "伺服器回應",
                        message: responseString,
                        preferredStyle: .alert
                    )
                    alert.addAction(UIAlertAction(title: "確定", style: .default))
                    // 取得前景中的 key window rootViewController
                    if let windowScene = UIApplication.shared.connectedScenes
                            .first(where: { $0.activationState == .foregroundActive }) as? UIWindowScene,
                       let rootVC = windowScene.windows.first(where: { $0.isKeyWindow })?.rootViewController {
                        rootVC.present(alert, animated: true, completion: nil)
                    }
                }
            }
            if !(200..<300).contains(http.statusCode) {
                print("🚫 伺服器錯誤狀態碼：\(http.statusCode)")
                completion(.failure(.serverError(statusCode: http.statusCode)))
                return
            }
            guard let data = data else {
                completion(.failure(.decodingError))
                return
            }
            do {
                let result = try JSONDecoder().decode(SignInResponse.self, from: data)
                print("✅ 登入成功，解析結果：\(result)")
                completion(.success(result))
            } catch {
                print("❗️ JSON 解碼失敗：\(error.localizedDescription)")
                completion(.failure(.decodingError))
            }
        }.resume()
    }
    
    // MARK: - URLSessionDelegate 備註：允許自簽憑證
    func urlSession(_ session: URLSession, didReceive challenge: URLAuthenticationChallenge,
                    completionHandler: @escaping (URLSession.AuthChallengeDisposition, URLCredential?) -> Void) {
        if challenge.protectionSpace.authenticationMethod == NSURLAuthenticationMethodServerTrust,
           let serverTrust = challenge.protectionSpace.serverTrust {
            let credential = URLCredential(trust: serverTrust)
            completionHandler(.useCredential, credential)
        } else {
            completionHandler(.performDefaultHandling, nil)
        }
    }
}
