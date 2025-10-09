import Foundation

/// ProfileViewModel 專注處理使用者帳號相關的操作流程。
@MainActor
final class ProfileViewModel: ObservableObject {
    @Published var isWorking: Bool = false
    @Published var message: String?
    @Published var showMessage: Bool = false
    @Published var lastRegisteredDevice: IOSRegisterResponse?
    @Published var lastOTPSetup: CreateOTPResponse?

    /// 使用者更新登入密碼。
    func updatePassword(currentPassword: String, newPassword: String) async -> Bool {
        isWorking = true
        defer { isWorking = false }

        do {
            _ = try await APIClient.shared.updateUserPassword(currentPassword: currentPassword, newPassword: newPassword)
            presentMessage("密碼已更新")
            return true
        } catch {
            presentMessage(error.localizedDescription)
            return false
        }
    }

    /// 向後端註冊 / 更新行動裝置推播資訊。
    func registerDevice(deviceId: String?, apnsToken: String, deviceName: String?) async -> Bool {
        isWorking = true
        defer { isWorking = false }

        do {
            let response = try await APIClient.shared.registerIOSDevice(deviceId: deviceId, apnsToken: apnsToken, deviceName: deviceName)
            lastRegisteredDevice = response
            presentMessage("裝置「\(response.deviceName ?? "未命名")」已更新推播設定")
            return true
        } catch {
            presentMessage(error.localizedDescription)
            return false
        }
    }

    /// 產生 OTP 秘鑰與備援碼。
    @discardableResult
    func enableOTP() async -> CreateOTPResponse? {
        isWorking = true
        defer { isWorking = false }

        do {
            let response = try await APIClient.shared.createOTP()
            lastOTPSetup = response
            presentMessage("已啟用 OTP")
            return response
        } catch {
            presentMessage(error.localizedDescription)
            return nil
        }
    }

    /// 顯示短暫提示以回饋使用者操作結果。
    func presentMessage(_ text: String) {
        message = text
        showMessage = true
        DispatchQueue.main.asyncAfter(deadline: .now() + 2) { [weak self] in
            self?.showMessage = false
        }
    }
}
