import Foundation

// MARK: - Error Code Reference

/// 集中定義 API 錯誤碼對應，讓 UI / ViewModel 能以一致訊息回饋使用者 (SRP)。
struct ApiErrorCode: Equatable, Hashable {
    let rawValue: String

    /// Human readable description surfaced to the UI.
    var message: String {
        ApiErrorCode.messages[rawValue] ?? "未知錯誤（\(rawValue)）"
    }

    /// Indicates whether the call succeeded.
    var isSuccess: Bool { rawValue == "0" }

    init(rawValue: String) {
        self.rawValue = rawValue
    }
}

extension ApiErrorCode: LosslessStringConvertible {
    init?(_ description: String) {
        self.init(rawValue: description)
    }

    var description: String { rawValue }
}

extension ApiErrorCode: Decodable {
    init(from decoder: Decoder) throws {
        let container = try decoder.singleValueContainer()
        if let rawCode = try? container.decode(String.self) {
            self.init(rawValue: rawCode)
            return
        }
        if let intCode = try? container.decode(Int.self) {
            self.init(rawValue: String(intCode))
            return
        }
        throw DecodingError.dataCorruptedError(in: container, debugDescription: "無法解析 error_code")
    }
}

extension ApiErrorCode: Encodable {
    func encode(to encoder: Encoder) throws {
        var container = encoder.singleValueContainer()
        try container.encode(rawValue)
    }
}

private extension ApiErrorCode {
    static let messages: [String: String] = [
        "0": "成功",
        "120": "Edge ID 格式錯誤",
        "121": "Edge 版本格式錯誤 (需為 X.Y.Z)",
        "122": "Edge 名稱為空白",
        "123": "Edge ID 不存在",
        "124": "Email 格式錯誤",
        "125": "Edge ID 為空",
        "126": "Access Token 失效",
        "127": "缺少 Access Token",
        "128": "帳號或密碼錯誤",
        "129": "Email 為空",
        "130": "密碼為空",
        "131": "使用者名稱為空",
        "132": "Refresh Token 失效",
        "133": "此 Email 已存在",
        "134": "此 Edge 已綁定",
        "135": "未綁定指定 Edge",
        "136": "尚未綁定任何 Edge",
        "137": "Refresh Token 為空",
        "138": "User name 為空",
        "139": "User name 過長",
        "140": "Edge name 過長",
        "141": "新密碼為空",
        "142": "使用者不存在",
        "143": "舊密碼錯誤",
        "144": "Edge 密碼為空",
        "145": "Edge 版本為空",
        "146": "Edge ID 已存在",
        "147": "Edge 密碼錯誤",
        "148": "新 Edge 密碼為空",
        "150": "此帳號已啟用二階段驗證",
        "151": "尚未啟用二階段驗證",
        "152": "OTP 或備援碼驗證失敗",
        "MISSING_AUTHORIZATION_HEADER": "缺少 Authorization 標頭",
        "INVALID_AUTHORIZATION_HEADER": "Authorization 標頭格式錯誤",
        "INVALID_TOKEN": "Access Token 驗證失敗",
        "IOS_DEVICE_NOT_FOUND": "找不到對應的 iOS 裝置",
        "APNS_TOKEN_IN_USE": "此 APNS Token 已綁定其他裝置",
        "USER_SERVICE_UNAVAILABLE": "使用者服務暫時無法使用"
    ]
}

// MARK: - API Error Plumbing

/// 將後端錯誤回傳模型化，方便解析出標準化訊息。
private struct ApiErrorPayload: Decodable {
    let errorCode: ApiErrorCode?
    let message: String?
    let error: String?
    let status: Int?
    let detail: String?
    let errors: [String: String]?
}

/// 網路層統一錯誤型別，隔離傳輸細節並讓 UI 只需面對語義化結果。
enum ApiError: Error, LocalizedError {
    case invalidURL(String)
    case missingToken
    case transport(Error)
    case decoding(Error)
    case http(status: Int, code: ApiErrorCode?, message: String?, payload: Data?)
    case invalidPayload(reason: String)

    var errorDescription: String? {
        switch self {
        case .invalidURL(let path):
            return "無效的伺服器位址：\(path)"
        case .missingToken:
            return "缺少存取權杖，請重新登入。"
        case .transport(let error):
            return error.localizedDescription
        case .decoding:
            return "資料解析失敗，請稍後再試。"
        case .http(_, let code, let message, _):
            if let code = code {
                return code.message
            }
            if let message = message, !message.isEmpty {
                return message
            }
            return "伺服器回傳未知錯誤"
        case .invalidPayload(let reason):
            return reason
        }
    }
}

// MARK: - Request Abstractions

/// 透過 type-erasure 將多樣化的請求物件轉為單一型別，減少泛型束縛 (OCP)。
struct AnyEncodable: Encodable {
    private let encodeClosure: (Encoder) throws -> Void

    init<T: Encodable>(_ value: T) {
        self.encodeClosure = value.encode
    }

    func encode(to encoder: Encoder) throws {
        try encodeClosure(encoder)
    }
}

/// 描述單一 API 行為的資料結構，將路徑、方法與權限整理於一處 (SRP)。
struct Endpoint<Response: Decodable> {
    let path: String
    let method: HTTPMethod
    let requiresAuth: Bool
    var headers: [String: String]
    var queryItems: [URLQueryItem]
    var body: AnyEncodable?

    init(
        path: String,
        method: HTTPMethod = .get,
        requiresAuth: Bool = true,
        headers: [String: String] = [:],
        queryItems: [URLQueryItem] = [],
        body: AnyEncodable? = nil
    ) {
        self.path = path
        self.method = method
        self.requiresAuth = requiresAuth
        self.headers = headers
        self.queryItems = queryItems
        self.body = body
    }
}

enum HTTPMethod: String {
    case get = "GET"
    case post = "POST"
    case put = "PUT"
    case delete = "DELETE"
    case patch = "PATCH"
}

// MARK: - API Client

/// 負責發送 HTTP 請求與解析回應的核心服務，提供其他模組依賴的穩定抽象 (DIP)。
final class APIClient {
    struct Configuration {
        let baseURL: URL

        static let `default` = Configuration(baseURL: URL(string: "https://api.redsafe-tw.com")!)
    }

    static let shared = APIClient()

    private let configuration: Configuration
    private let session: URLSession
    private let tokenProvider: () async -> String?
    private let encoder: JSONEncoder
    private let decoder: JSONDecoder

    init(
        configuration: Configuration = .default,
        session: URLSession = .shared,
        tokenProvider: (() async -> String?)? = nil
    ) {
        self.configuration = configuration
        self.session = session
        self.tokenProvider = tokenProvider ?? {
            await MainActor.run { AuthManager.shared.accessToken }
        }

        let encoder = JSONEncoder()
        encoder.keyEncodingStrategy = .convertToSnakeCase
        self.encoder = encoder

        let decoder = JSONDecoder()
        decoder.keyDecodingStrategy = .convertFromSnakeCase
        self.decoder = decoder
    }

    /// 依據 Endpoint 發送請求並解析為對應模型，維持清晰的資料流與錯誤拋出策略。
    func send<Response: Decodable>(_ endpoint: Endpoint<Response>) async throws -> Response {
        let request = try await makeRequest(from: endpoint)

        do {
            let (data, response) = try await session.data(for: request)
            guard let httpResponse = response as? HTTPURLResponse else {
                throw ApiError.http(status: -1, code: nil, message: "無效的伺服器回應", payload: data)
            }

#if DEBUG
            debugLogResponse(for: request, response: httpResponse, data: data)
#endif

            let statusCode = httpResponse.statusCode

            guard (200...299).contains(statusCode) else {
                throw parseError(status: statusCode, data: data)
            }

            if Response.self == EmptyPayload.self {
                return EmptyPayload() as! Response
            }

            guard !data.isEmpty else {
                throw ApiError.http(status: statusCode, code: nil, message: "伺服器未回傳資料", payload: data)
            }

            do {
                return try decoder.decode(Response.self, from: data)
            } catch {
#if DEBUG
                print("❗️ Decoding \(Response.self) 失敗：\(error)")
                if let body = String(data: data, encoding: .utf8) {
                    print("   ↳ 原始 Body: \(body)")
                }
#endif
                throw ApiError.decoding(error)
            }
        } catch let error as ApiError {
            throw error
        } catch {
            throw ApiError.transport(error)
        }
    }

    private func makeRequest<Response>(from endpoint: Endpoint<Response>) async throws -> URLRequest {
        guard var components = URLComponents(url: configuration.baseURL, resolvingAgainstBaseURL: false) else {
            throw ApiError.invalidURL(configuration.baseURL.absoluteString)
        }

        let normalizedPath = endpoint.path.hasPrefix("/") ? endpoint.path : "/" + endpoint.path
        components.path = configuration.baseURL.path + normalizedPath
        components.queryItems = endpoint.queryItems.isEmpty ? nil : endpoint.queryItems

        guard let url = components.url else {
            throw ApiError.invalidURL(normalizedPath)
        }

        var request = URLRequest(url: url)
        request.httpMethod = endpoint.method.rawValue
        request.setValue("application/json", forHTTPHeaderField: "Accept")

        if let body = endpoint.body {
            do {
                request.httpBody = try encoder.encode(body)
                request.setValue("application/json", forHTTPHeaderField: "Content-Type")
            } catch {
                throw ApiError.invalidPayload(reason: "送出資料格式有誤：\(error.localizedDescription)")
            }
        }

        for (key, value) in endpoint.headers {
            request.setValue(value, forHTTPHeaderField: key)
        }

        if endpoint.requiresAuth {
            let token = await tokenProvider()
            guard let token, !token.isEmpty else {
                throw ApiError.missingToken
            }
            request.setValue("Bearer \(token)", forHTTPHeaderField: "Authorization")
        }

        return request
    }

    private func parseError(status: Int, data: Data?) -> ApiError {
        guard let data, !data.isEmpty else {
            return .http(status: status, code: nil, message: nil, payload: data)
        }

        if let payload = try? decoder.decode(ApiErrorPayload.self, from: data) {
            let code = payload.errorCode
                ?? payload.message.flatMap(ApiErrorCode.init(rawValue:))
                ?? payload.error.flatMap(ApiErrorCode.init(rawValue:))

            let message = payload.detail
                ?? payload.error
                ?? payload.message
                ?? payload.errors?.map { "\($0.key)：\($0.value)" }.joined(separator: "\n")

            return .http(status: status, code: code, message: message, payload: data)
        }

        if let rawText = String(data: data, encoding: .utf8), !rawText.isEmpty {
            return .http(status: status, code: nil, message: rawText, payload: data)
        }

        return .http(status: status, code: nil, message: nil, payload: data)
    }
}

#if DEBUG
extension APIClient {
    private func debugLogResponse(for request: URLRequest, response: HTTPURLResponse, data: Data) {
        let urlString = request.url?.absoluteString ?? "<unknown URL>"
        let method = request.httpMethod ?? "<NO METHOD>"
        print("\n📬 [API] Response \(response.statusCode) ← \(method) \(urlString)")

        if !response.allHeaderFields.isEmpty {
            let headerLines = response.allHeaderFields
                .map { "  \($0.key): \($0.value)" }
                .sorted()
                .joined(separator: "\n")
            print("🔖 Headers:\n\(headerLines)")
        } else {
            print("🔖 Headers: <none>")
        }

        if data.isEmpty {
            print("📦 Body: <empty>")
            return
        }

        if let prettyJSON = prettyPrintedJSON(from: data) {
            print("📦 Body (JSON):\n\(prettyJSON)")
        } else if let bodyString = String(data: data, encoding: .utf8) {
            print("📦 Body (UTF-8):\n\(bodyString)")
        } else {
            print("📦 Body: <non-UTF8 binary, \(data.count) bytes>")
        }
    }

    private func prettyPrintedJSON(from data: Data) -> String? {
        guard
            let object = try? JSONSerialization.jsonObject(with: data, options: []),
            JSONSerialization.isValidJSONObject(object),
            let prettyData = try? JSONSerialization.data(withJSONObject: object, options: [.prettyPrinted]),
            let prettyString = String(data: prettyData, encoding: .utf8)
        else {
            return nil
        }
        return prettyString
    }
}
#endif

// MARK: - Request / Response DTOs

struct SignInRequest: Encodable {
    let email: String
    let password: String
}

struct SignInOTPRequest: Encodable {
    let email: String
    let password: String
    let otpCode: String?
    let backupCode: String?
}

struct SignUpRequest: Encodable {
    let email: String
    let userName: String
    let password: String
}

struct RefreshRequest: Encodable {
    let refreshToken: String
}

struct IOSRegisterRequest: Encodable {
    let iosDeviceId: String?
    let apnsToken: String
    let deviceName: String?
}

struct BindEdgeRequest: Encodable {
    let edgeId: String
    let edgeName: String
    let edgePassword: String
}

struct UpdateEdgeNameRequest: Encodable {
    let edgeId: String
    let edgeName: String
}

struct UpdateUserNameRequest: Encodable {
    let userName: String
}

struct UpdateUserPasswordRequest: Encodable {
    let password: String
    let newPassword: String
}

struct UpdateEdgePasswordRequest: Encodable {
    let edgeId: String
    let edgePassword: String
    let newEdgePassword: String
}

struct SignInResponse: Decodable {
    let userName: String?
    let accessToken: String?
    let refreshToken: String?
    let errorCode: ApiErrorCode?

    var normalizedUserName: String? {
        guard let name = userName?.trimmingCharacters(in: .whitespacesAndNewlines), !name.isEmpty else {
            return nil
        }
        return name
    }

    var requiresOTP: Bool { errorCode?.rawValue == "150" }
}

struct CreateOTPResponse: Decodable {
    let otpKey: String
    let backupCodes: [String]
}

struct SignUpResponse: Decodable {
    let userId: String
    let userName: String
}

struct RefreshResponse: Decodable {
    let accessToken: String
}

struct EdgeSummary: Decodable, Identifiable, Hashable {
    let edgeId: String
    let displayName: String
    let isOnline: Bool?

    var id: String { edgeId }

    init(edgeId: String, displayName: String, isOnline: Bool? = nil) {
        self.edgeId = edgeId
        self.displayName = displayName
        self.isOnline = isOnline
    }
}

struct EdgeListResponse: Decodable {
    let edges: [EdgeSummary]
}

struct ErrorCodeResponse: Decodable {
    let errorCode: ApiErrorCode
}

struct IOSRegisterResponse: Decodable {
    let iosDeviceId: String
    let apnsToken: String
    let deviceName: String?
}

struct EmptyPayload: Decodable { }

// MARK: - High level API surface

extension APIClient {
    func signIn(email: String, password: String) async throws -> SignInResponse {
        let payload = SignInRequest(email: email, password: password)
        let endpoint = Endpoint<SignInResponse>(
            path: "/auth/signin",
            method: .post,
            requiresAuth: false,
            body: AnyEncodable(payload)
        )
        return try await send(endpoint)
    }

    func signInWithOTP(email: String, password: String, otpCode: String?, backupCode: String?) async throws -> SignInResponse {
        let payload = SignInOTPRequest(email: email, password: password, otpCode: otpCode, backupCode: backupCode)
        let endpoint = Endpoint<SignInResponse>(
            path: "/auth/signin/otp",
            method: .post,
            requiresAuth: false,
            body: AnyEncodable(payload)
        )
        return try await send(endpoint)
    }

    func createOTP() async throws -> CreateOTPResponse {
        let endpoint = Endpoint<CreateOTPResponse>(
            path: "/auth/create/otp",
            method: .post
        )
        return try await send(endpoint)
    }

    func signUp(email: String, userName: String, password: String) async throws -> SignUpResponse {
        let payload = SignUpRequest(email: email, userName: userName, password: password)
        let endpoint = Endpoint<SignUpResponse>(
            path: "/auth/signup",
            method: .post,
            requiresAuth: false,
            body: AnyEncodable(payload)
        )
        return try await send(endpoint)
    }

    func refreshAccessToken(refreshToken: String) async throws -> RefreshResponse {
        let payload = RefreshRequest(refreshToken: refreshToken)
        let endpoint = Endpoint<RefreshResponse>(
            path: "/auth/refresh",
            method: .post,
            requiresAuth: false,
            body: AnyEncodable(payload)
        )
        return try await send(endpoint)
    }

    func fetchEdgeList() async throws -> [EdgeSummary] {
        let endpoint = Endpoint<EdgeListResponse>(path: "/user/list/edge_id")
        do {
            let response = try await send(endpoint)
            return response.edges
        } catch ApiError.http(_, let code?, _, _) where code.rawValue == "136" {
            return []
        } catch {
            throw error
        }
    }

    func bindEdge(edgeId: String, displayName: String, edgePassword: String) async throws -> ApiErrorCode {
        let payload = BindEdgeRequest(edgeId: edgeId, edgeName: displayName, edgePassword: edgePassword)
        let endpoint = Endpoint<ErrorCodeResponse>(
            path: "/user/bind",
            method: .post,
            body: AnyEncodable(payload)
        )
        let response = try await send(endpoint)
        return try mapSuccess(from: response)
    }

    func unbindEdge(edgeId: String) async throws -> ApiErrorCode {
        let endpoint = Endpoint<ErrorCodeResponse>(
            path: "/user/unbind/\(edgeId)",
            method: .post
        )
        let response = try await send(endpoint)
        return try mapSuccess(from: response)
    }

    func updateEdgeName(edgeId: String, newName: String) async throws -> ApiErrorCode {
        let payload = UpdateEdgeNameRequest(edgeId: edgeId, edgeName: newName)
        let endpoint = Endpoint<ErrorCodeResponse>(
            path: "/user/update/edge_name",
            method: .post,
            body: AnyEncodable(payload)
        )
        let response = try await send(endpoint)
        return try mapSuccess(from: response)
    }

    func updateUserName(_ newName: String) async throws -> ApiErrorCode {
        let payload = UpdateUserNameRequest(userName: newName)
        let endpoint = Endpoint<ErrorCodeResponse>(
            path: "/user/update/user_name",
            method: .post,
            body: AnyEncodable(payload)
        )
        let response = try await send(endpoint)
        return try mapSuccess(from: response)
    }

    func updateUserPassword(currentPassword: String, newPassword: String) async throws -> ApiErrorCode {
        let payload = UpdateUserPasswordRequest(password: currentPassword, newPassword: newPassword)
        let endpoint = Endpoint<ErrorCodeResponse>(
            path: "/user/update/password",
            method: .post,
            body: AnyEncodable(payload)
        )
        let response = try await send(endpoint)
        return try mapSuccess(from: response)
    }

    func updateEdgePassword(edgeId: String, currentPassword: String, newPassword: String) async throws -> ApiErrorCode {
        let payload = UpdateEdgePasswordRequest(edgeId: edgeId, edgePassword: currentPassword, newEdgePassword: newPassword)
        let endpoint = Endpoint<ErrorCodeResponse>(
            path: "/user/update/edge_password",
            method: .post,
            body: AnyEncodable(payload)
        )
        let response = try await send(endpoint)
        return try mapSuccess(from: response)
    }

    func registerIOSDevice(deviceId: String?, apnsToken: String, deviceName: String?) async throws -> IOSRegisterResponse {
        let payload = IOSRegisterRequest(iosDeviceId: deviceId, apnsToken: apnsToken, deviceName: deviceName)
        let endpoint = Endpoint<IOSRegisterResponse>(
            path: "/ios/reg",
            method: .post,
            body: AnyEncodable(payload)
        )
        return try await send(endpoint)
    }

    private func mapSuccess(from response: ErrorCodeResponse) throws -> ApiErrorCode {
        if response.errorCode.isSuccess {
            return response.errorCode
        }
        throw ApiError.http(status: 200, code: response.errorCode, message: nil, payload: nil)
    }
}
