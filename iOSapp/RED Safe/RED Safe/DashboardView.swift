import SwiftUI
import UIKit

// MARK: - 裝置分頁

/// Sheet 狀態列舉：新增、改名、更新密碼三種情境。
enum DeviceSheet: Identifiable {
    case bind
    case rename(EdgeSummary)
    case password(EdgeSummary)

    var id: String {
        switch self {
        case .bind: return "bind"
        case .rename(let edge): return "rename-\(edge.edgeId)"
        case .password(let edge): return "password-\(edge.edgeId)"
        }
    }
}

/// DashboardView 管理 Edge 裝置相關 UI 與互動。
struct DashboardView: View {
    @ObservedObject var homeVM: HomeViewModel
    @ObservedObject var auth: AuthManager

    @Binding var deviceSheet: DeviceSheet?
    @Binding var deviceToUnbind: EdgeSummary?
    @Binding var showUnbindConfirm: Bool
    @Binding var animateBackground: Bool

    @State private var navigationPath = NavigationPath()

    var body: some View {
        NavigationStack(path: $navigationPath) {
            GeometryReader { proxy in
                ZStack {
                    background

                    VStack(spacing: 20) {
                        header
                            .padding(.top, proxy.safeAreaInsets.top + 6)

                        summaryCard

                        deviceContent
                            .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .top)
                    }
                    .padding(.horizontal, 20)
                    .padding(.bottom, 24 + proxy.safeAreaInsets.bottom)
                    .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .top)
                }
                .ignoresSafeArea()
                .onAppear { animateBackground = true }
            }
            .navigationDestination(for: EdgeSummary.self) { edge in
                DeviceDetailView(
                    edge: edge,
                    rename: { deviceSheet = .rename(edge) },
                    updatePassword: { deviceSheet = .password(edge) },
                    unbind: {
                        deviceToUnbind = edge
                        showUnbindConfirm = true
                    }
                )
            }
        }
        .sheet(item: $deviceSheet) { sheet in
            switch sheet {
            case .bind:
                BindEdgeSheet { edgeId, name, password in
                    homeVM.bindEdge(edgeId: edgeId, name: name, password: password)
                }
            case .rename(let edge):
                RenameEdgeSheet(edge: edge) { newName in
                    homeVM.renameEdge(edgeId: edge.edgeId, newName: newName)
                }
            case .password(let edge):
                EdgePasswordSheet(edge: edge) { current, newPassword in
                    homeVM.updateEdgePassword(edgeId: edge.edgeId, currentPassword: current, newPassword: newPassword)
                }
            }
        }
        .confirmationDialog(
            "確定要解除綁定這台 Edge 嗎？",
            isPresented: $showUnbindConfirm,
            titleVisibility: .visible
        ) {
            Button("解除綁定", role: .destructive) {
                guard let deviceToUnbind else { return }
                homeVM.unbindEdge(edgeId: deviceToUnbind.edgeId)
                self.deviceToUnbind = nil
            }
            Button("取消", role: .cancel) {
                deviceToUnbind = nil
            }
        } message: {
            Text("解除綁定後，需要重新輸入 Edge 密碼才能再次連結。")
        }
    }

    // MARK: - 子區塊

    private var background: some View {
        ZStack {
            LinearGradient(
                colors: [
                    Color(red: 246/255, green: 250/255, blue: 255/255),
                    Color(red: 223/255, green: 235/255, blue: 250/255),
                    Color(red: 205/255, green: 222/255, blue: 245/255)
                ],
                startPoint: .topLeading,
                endPoint: .bottomTrailing
            )
            .ignoresSafeArea()

            Circle()
                .fill(Color.white.opacity(0.6))

            Color(.systemGroupedBackground)
                .ignoresSafeArea()

            Circle()
                .fill(Color.white.opacity(0.45))
                .frame(width: animateBackground ? 360 : 220)
                .blur(radius: 60)
                .offset(x: -150, y: animateBackground ? -260 : -140)
                .animation(.easeOut(duration: 1.0), value: animateBackground)

            Circle()
                .fill(Color.white.opacity(0.32))
                .frame(width: animateBackground ? 320 : 200)
                .blur(radius: 52)
                .offset(x: 170, y: animateBackground ? 280 : 160)
                .animation(.easeOut(duration: 1.0).delay(0.05), value: animateBackground)
        }
    }

    private var header: some View {
        HStack(spacing: 16) {
            VStack(alignment: .leading, spacing: 8) {
                Text("你好，\(auth.userName ?? "使用者")")
                    .font(.system(size: 30, weight: .bold, design: .rounded))
                    .foregroundColor(.primary)
                Text("管理並監控已綁定的 Edge 裝置")
                    .font(.subheadline)
                    .foregroundColor(.secondary)
            }
            Spacer()
            quickActionButton(icon: "arrow.clockwise") {
                homeVM.loadEdges(showIndicator: true)
            }
        }
    }

    private var summaryCard: some View {
        let count = homeVM.edges.count

        return HStack(spacing: 16) {
            VStack(alignment: .leading, spacing: 10) {
                Text("已綁定 Edge")
                    .font(.caption.weight(.semibold))
                    .foregroundStyle(Color.secondary)
                HStack(alignment: .firstTextBaseline, spacing: 8) {
                    Text("\(count)")
                        .font(.system(size: 44, weight: .bold, design: .rounded))
                    Text("台")
                        .font(.title3)
                        .foregroundStyle(Color.secondary)
                }
                Text(count == 0 ? "尚未綁定任何 Edge 裝置" : "點選下方裝置卡片以快速管理 Edge")
                    .font(.footnote)
                    .foregroundStyle(Color.secondary)
            }
            .frame(maxWidth: .infinity, alignment: .leading)

            Button {
                deviceSheet = .bind
            } label: {
                Image(systemName: "plus")
                    .font(.headline)
                    .padding(.horizontal, 18)
                    .padding(.vertical, 10)
                    .background(
                        Capsule().fill(
                            LinearGradient(
                                colors: [
                                    Color(red: 118/255, green: 186/255, blue: 255/255),
                                    Color(red: 78/255, green: 156/255, blue: 255/255)
                                ],
                                startPoint: .topLeading,
                                endPoint: .bottomTrailing
                            )
                        )
                    )
                    .overlay(
                        Capsule()
                            .stroke(Color.white.opacity(0.55))
                    )
                    .foregroundStyle(Color.white)
            }
            .buttonStyle(.plain)
        }
        .padding(24)
        .glassCard(cornerRadius: 30)
        .padding(.horizontal, 4)
    }

    private var deviceContent: some View {
        Group {
            if homeVM.isLoading && homeVM.edges.isEmpty {
                ProgressView()
                    .progressViewStyle(.circular)
                    .tint(.white)
                    .padding(.top, 40)
            } else if homeVM.edges.isEmpty {
                emptyState
            } else {
                ScrollView(showsIndicators: false) {
                    LazyVStack(spacing: 18) {
                        ForEach(homeVM.edges) { edge in
                            NavigationLink(value: edge) {
                                EdgeCard(edge: edge)
                            }
                            .buttonStyle(.plain)
                        }
                    }
                    .padding(.bottom, 32)
                }
                .refreshable {
                    homeVM.loadEdges(showIndicator: false)
                }
            }
        }
    }

    private var emptyState: some View {
        VStack(spacing: 18) {
            Image(systemName: "shippingbox.fill")
                .font(.system(size: 60))
                .foregroundStyle(Color.accentColor)
            Text("還沒有綁定任何 Edge 裝置")
                .font(.headline)
                .foregroundStyle(Color.primary)
            Text("點擊下方按鈕開始綁定 Edge 裝置")
                .font(.footnote)
                .foregroundStyle(Color.secondary)
                .multilineTextAlignment(.center)
                .padding(.horizontal, 12)
            Button {
                deviceSheet = .bind
            } label: {
                Text("立即綁定")
                    .font(.headline)
                    .foregroundStyle(Color.white)
                    .padding(.horizontal, 32)
                    .padding(.vertical, 12)
                    .background(
                        Capsule().fill(
                            LinearGradient(
                                colors: [
                                    Color(red: 118/255, green: 186/255, blue: 255/255),
                                    Color(red: 78/255, green: 156/255, blue: 255/255)
                                ],
                                startPoint: .topLeading,
                                endPoint: .bottomTrailing
                            )
                        )
                    )
                    .overlay(Capsule().stroke(Color.white.opacity(0.6)))
            }
            .buttonStyle(.plain)
        }
        .padding(36)
        .frame(maxWidth: .infinity)
        .background(
            RoundedRectangle(cornerRadius: 32, style: .continuous)
                .fill(Color.white.opacity(0.92))
        )
        .overlay(
            RoundedRectangle(cornerRadius: 32, style: .continuous)
                .stroke(Color.white.opacity(0.4))
        )
        .shadow(color: Color.black.opacity(0.08), radius: 14, x: 0, y: 10)
    }

    private func quickActionButton(icon: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Image(systemName: icon)
                .font(.headline)
                .foregroundStyle(Color.accentColor)
                .frame(width: 44, height: 44)
                .background(Circle().fill(Color.white.opacity(0.9)))
        }
        .buttonStyle(.plain)
        .shadow(color: Color.black.opacity(0.1), radius: 6, x: 0, y: 4)
    }
}

// MARK: - 元件

private struct EdgeCard: View {
    let edge: EdgeSummary

    var body: some View {
        let online = edge.isOnline ?? false
        let statusColor = online ? Color.green.opacity(0.85) : Color.red.opacity(0.8)
        let statusText = online ? "在線" : "離線"
        let statusIcon = online ? "wifi" : "wifi.slash"

        return VStack(alignment: .leading, spacing: 18) {
            HStack(alignment: .top, spacing: 16) {
                LinearGradient(
                    colors: [
                        Color(red: 0/255, green: 187/255, blue: 255/255),
                        Color(red: 0/255, green: 120/255, blue: 255/255)
                    ],
                    startPoint: .topLeading,
                    endPoint: .bottomTrailing
                )
                .frame(width: 58, height: 58)
                .clipShape(RoundedRectangle(cornerRadius: 18, style: .continuous))
                .shadow(color: Color.black.opacity(0.18), radius: 10, x: 0, y: 8)
                .overlay(
                    Image(systemName: "dot.radiowaves.left.and.right")
                        .foregroundStyle(.white)
                        .font(.title2)
                )
                VStack(alignment: .leading, spacing: 10) {
                    Text(edge.displayName.isEmpty ? "未命名裝置" : edge.displayName)
                        .font(.system(size: 22, weight: .semibold, design: .rounded))
                        .foregroundStyle(Color.primary)
                    Text(edge.edgeId)
                        .font(.footnote.monospaced())
                        .foregroundStyle(Color.secondary)
                }
                Spacer()
            }
            Divider()
                .background(Color.gray.opacity(0.15))
            HStack(spacing: 8) {
                Image(systemName: statusIcon)
                    .foregroundStyle(statusColor)
                Text(statusText)
                    .font(.footnote.weight(.medium))
                    .foregroundStyle(statusColor)
            }
            .padding(.top, 4)
        }
        .padding(22)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(
            RoundedRectangle(cornerRadius: 30, style: .continuous)
                .fill(
                    LinearGradient(
                        colors: [Color.white.opacity(0.90), Color.white.opacity(0.88)],
                        startPoint: .topLeading,
                        endPoint: .bottomTrailing
                    )
                )
        )
        .overlay(
            RoundedRectangle(cornerRadius: 30, style: .continuous)
                .stroke(Color.white.opacity(0.28), lineWidth: 1)
        )
        .padding(.horizontal, 4)
    }
}

// MARK: - 互動表單
private struct BindEdgeSheet: View {
    @Environment(\.dismiss) private var dismiss
    @State private var edgeId = ""
    @State private var displayName = ""
    @State private var edgePassword = ""
    @State private var scanError: String? = nil

    @State private var showScanner = false
    @State private var showPhotoPicker = false
    @State private var torchOn = false
    @State private var showPermissionAlert = false
    @State private var permissionAlertMessage = ""

    // 正規表達式：擷取 Edge ID 與密碼
    private let edgeIdRegex = try! NSRegularExpression(pattern: "RED-[A-F0-9]{8}")
    private let passwordRegex = try! NSRegularExpression(pattern: "(?:(?:PWD|PASS|PASSWORD)[:=]\\s*)([^\\n\\r\\t ]{1,64})", options: [.caseInsensitive])

    var onSubmit: (String, String, String) -> Void

    private var normalizedEdgeId: String { edgeId.trimmed.uppercased() }
    private var isEdgeIdValid: Bool {
        let pattern = "^RED-[A-F0-9]{8}$"
        return NSPredicate(format: "SELF MATCHES %@", pattern).evaluate(with: normalizedEdgeId)
    }
    private var isNameValid: Bool { !displayName.trimmed.isEmpty && displayName.trimmed.count <= 16 }
    private var isFormValid: Bool { isEdgeIdValid && isNameValid && !edgePassword.trimmed.isEmpty }

    // 支援 JSON 掃描內容（容忍不同鍵名）
    private struct QRPayload: Decodable {
        let serial: String?
        let edge_id: String?
        let edgeId: String?
        let password: String?
        let pass: String?
        let pwd: String?
        // （可選）顯示名稱（如果你需要新的鍵名，直接在這裡加欄位即可）
        let name: String?
    }

    /// 嘗試將 JSON 內容帶入欄位；成功時回傳 true
    private func tryAutofillFromJSON(_ payload: String) -> Bool {
        guard let data = payload.data(using: .utf8) else { return false }
        do {
            let obj = try JSONDecoder().decode(QRPayload.self, from: data)
            let id = obj.serial ?? obj.edge_id ?? obj.edgeId
            let pw = obj.password ?? obj.pass ?? obj.pwd

            var filled = false
            if let id, !id.isEmpty {
                edgeId = id.uppercased()
                filled = true
            }
            if let pw, !pw.isEmpty {
                edgePassword = pw
                filled = true
            }
            if let name = obj.name?.trimmingCharacters(in: .whitespacesAndNewlines), !name.isEmpty {
                // 套用現有限制（最長 16）
                displayName = String(name.prefix(16))
                filled = true
            }
            return filled
        } catch {
            return false
        }
    }

    // 從QR字串自動帶入欄位（優先解析 JSON；失敗則回退既有格式）
    private func autofill(from payload: String) {
        let beforeId = edgeId
        let beforePw = edgePassword
        let beforeName = displayName

        // 1) 先試著把 JSON 格式帶入
        if tryAutofillFromJSON(payload) {
            // 若完全沒有帶入任何欄位，提示使用者
            if edgeId == beforeId && edgePassword == beforePw && displayName == beforeName {
                scanError = "無法從掃描內容辨識出有效的 JSON 或欄位。\n\n請確認 QR 內容包含例如：{\"serial\":\"RED-XXXXXXXX\",\"password\":\"******\"}"
            }
            return
        }

        // 2) 回退：相容舊版「文字/鍵值」格式
        let fullRange = NSRange(location: 0, length: (payload as NSString).length)
        if let idMatch = edgeIdRegex.firstMatch(in: payload, options: [], range: fullRange) {
            if let range = Range(idMatch.range, in: payload) {
                edgeId = String(payload[range]).uppercased()
            }
        }
        if let passMatch = passwordRegex.firstMatch(in: payload, options: [], range: fullRange) {
            if passMatch.numberOfRanges > 1, let range = Range(passMatch.range(at: 1), in: payload) {
                edgePassword = String(payload[range])
            }
        }
        // 若字串只包含 Edge ID (無密碼)，也盡量抓出
        if edgePassword.isEmpty {
            // 嘗試用逗號或換行分割並找像密碼的片段
            let parts = payload
                .replacingOccurrences(of: "\r", with: "")
                .components(separatedBy: CharacterSet(charactersIn: ",\n\t "))
            if let candidate = parts.first(where: { !$0.isEmpty && !$0.uppercased().hasPrefix("RED-") }) {
                edgePassword = candidate
            }
        }
        // 若完全沒有帶入任何欄位，提示使用者
        if edgeId == beforeId && edgePassword == beforePw && displayName == beforeName {
            scanError = "無法從掃描內容辨識出有效的 JSON 或欄位。\n\n請確認 QR 內容包含例如：{\"serial\":\"RED-XXXXXXXX\",\"password\":\"******\"}"
        }
    }

    // 要求相機權限後再開啟掃描
    private func ensureCameraAuthorized(then action: @escaping () -> Void) {
        switch AVCaptureDevice.authorizationStatus(for: .video) {
        case .authorized:
            action()
        case .notDetermined:
            AVCaptureDevice.requestAccess(for: .video) { granted in
                DispatchQueue.main.async {
                    if granted { action() }
                    else {
                        permissionAlertMessage = "沒有相機權限，無法進行掃描。請到「設定 > 隱私權 > 相機」開啟。"
                        showPermissionAlert = true
                    }
                }
            }
        case .denied, .restricted:
            permissionAlertMessage = "沒有相機權限，無法進行掃描。請到「設定 > 隱私權 > 相機」開啟。"
            showPermissionAlert = true
        @unknown default:
            permissionAlertMessage = "相機權限狀態未知，請稍後再試。"
            showPermissionAlert = true
        }
    }

    // （可選）檢查相簿權限；使用 PHPicker 一般不需權限，但有些機型/配置可能要求
    private func ensurePhotoAuthorized(then action: @escaping () -> Void) {
        let status = PHPhotoLibrary.authorizationStatus(for: .readWrite)
        switch status {
        case .authorized, .limited:
            action()
        case .notDetermined:
            PHPhotoLibrary.requestAuthorization(for: .readWrite) { newStatus in
                DispatchQueue.main.async {
                    if newStatus == .authorized || newStatus == .limited { action() }
                    else {
                        permissionAlertMessage = "沒有相簿權限，無法開啟照片庫。請到「設定 > 隱私權 > 照片」開啟。"
                        showPermissionAlert = true
                    }
                }
            }
        case .denied, .restricted:
            permissionAlertMessage = "沒有相簿權限，無法開啟照片庫。請到「設定 > 隱私權 > 照片」開啟。"
            showPermissionAlert = true
        @unknown default:
            permissionAlertMessage = "相簿權限狀態未知，請稍後再試。"
            showPermissionAlert = true
        }
    }

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    TextField("Edge ID (RED-XXXXXXXX)", text: $edgeId)
                        .textInputAutocapitalization(.characters)
                        .autocorrectionDisabled()
                        .onChangeCompat(of: edgeId) { _, newValue in
                            if newValue != newValue.uppercased() {
                                edgeId = newValue.uppercased()
                            }
                        }
                    TextField("顯示名稱", text: $displayName)
                        .onChangeCompat(of: displayName) { _, value in
                            if value.count > 16 {
                                displayName = String(value.prefix(16))
                            }
                        }
                    SecureField("Edge 密碼", text: $edgePassword)
                } header: {
                    HStack {
                        Text("Edge 資訊")
                        Spacer()
                        // 右上角小相機按鈕（僅圖示，點擊開啟掃描）
                        Button {
                            ensureCameraAuthorized { showScanner = true }
                        } label: {
                            Image(systemName: "qrcode.viewfinder")
                                .font(.title3)
                                .imageScale(.large)
                        }
                        .buttonStyle(.plain)
                    }
                }
                if !isEdgeIdValid {
                    Text("Edge ID 必須為 RED- 開頭的 8 碼十六進位字元")
                        .font(.footnote)
                        .foregroundColor(.red)
                }
                if !isNameValid {
                    Text("裝置名稱需為 1-16 字元")
                        .font(.footnote)
                        .foregroundColor(.red)
                }
            }
            .navigationTitle("綁定 Edge")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("取消") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("綁定") {
                        onSubmit(normalizedEdgeId, displayName.trimmed, edgePassword.trimmed)
                        dismiss()
                    }
                    .disabled(!isFormValid)
                }
            }
        }
        .sheet(isPresented: $showScanner) {
            QRScannerSheet(torchOn: $torchOn) { payload in
                autofill(from: payload)
            }
        }
        .alert("權限需要", isPresented: $showPermissionAlert) {
            Button("知道了", role: .cancel) {}
        } message: {
            Text(permissionAlertMessage)
        }
        .alert("掃描內容無法解析", isPresented: .constant(scanError != nil)) {
            Button("知道了") { scanError = nil }
        } message: {
            Text(scanError ?? "")
        }
    }
}

// MARK: - QR 掃描 Sheet（含相機預覽、動畫、手電筒、圖庫）
import AVFoundation
import PhotosUI
import Vision

private struct QRScannerSheet: View {
    @Environment(\.dismiss) private var dismiss
    @Binding var torchOn: Bool
    var onFound: (String) -> Void

    @State private var lastPayload: String = ""
    @State private var showPhotoPicker = false
    @State private var showPermissionAlert = false
    @State private var permissionAlertMessage = ""

    private func ensurePhotoAuthorized(then action: @escaping () -> Void) {
        let status = PHPhotoLibrary.authorizationStatus(for: .readWrite)
        switch status {
        case .authorized, .limited:
            action()
        case .notDetermined:
            PHPhotoLibrary.requestAuthorization(for: .readWrite) { newStatus in
                DispatchQueue.main.async {
                    if newStatus == .authorized || newStatus == .limited { action() }
                    else {
                        permissionAlertMessage = "沒有相簿權限，無法開啟照片庫。請到「設定 > 隱私權 > 照片」開啟。"
                        showPermissionAlert = true
                    }
                }
            }
        case .denied, .restricted:
            permissionAlertMessage = "沒有相簿權限，無法開啟照片庫。請到「設定 > 隱私權 > 照片」開啟。"
            showPermissionAlert = true
        @unknown default:
            permissionAlertMessage = "相簿權限狀態未知，請稍後再試。"
            showPermissionAlert = true
        }
    }

    var body: some View {
        ZStack {
            CameraPreview(torchOn: $torchOn) { value in
                guard value != lastPayload else { return }
                lastPayload = value
                onFound(value)
                // 自動關閉掃描畫面
                dismiss()
            }
            .background(Color.clear)
            .ignoresSafeArea()

            // 掃描框與動畫（置中）
            ScannerOverlay()
                .allowsHitTesting(false)

            // 底部控制列
            VStack {
                Spacer()
                HStack {
                    // 手電筒：點一下就開（若已開則維持開啟；再次點擊可關閉）
                    Button {
                        torchOn.toggle()
                    } label: {
                        EmptyView()
                    }
                    .buttonStyle(LiquidIconButtonStyle(systemName: torchOn ? "flashlight.on.fill" : "flashlight.off.fill", isActive: torchOn))

                    Spacer()

                    // 圖庫：開啟相簿挑選 QR 照片
                    Button {
                        ensurePhotoAuthorized { showPhotoPicker = true }
                    } label: {
                        EmptyView()
                    }
                    .buttonStyle(LiquidIconButtonStyle(systemName: "photo.on.rectangle", isActive: false))
                }
                .padding(.horizontal, 24)
                .padding(.bottom, 24)
            }
        }
        .sheet(isPresented: $showPhotoPicker) {
            QRPhotoPicker { payload in
                onFound(payload)
                dismiss()
            }
        }
        .alert("權限需要", isPresented: $showPermissionAlert) {
            Button("知道了", role: .cancel) {}
        } message: {
            Text(permissionAlertMessage)
        }
        .onDisappear {
            // Stop the capture session when leaving
            torchOn = false
            // Access coordinator and stop session
            // Using NotificationCenter to broadcast stop
            NotificationCenter.default.post(name: .stopCameraSession, object: nil)
        }
    }
}

private struct ScannerOverlay: View {
    @State private var progress: CGFloat = 0
    var body: some View {
        GeometryReader { geo in
            let width = min(geo.size.width * 0.8, 320)
            let height = width
            let top = (geo.size.height - height) / 2.8

            ZStack {
                RoundedRectangle(cornerRadius: 16)
                    .strokeBorder(.white.opacity(0.9), lineWidth: 2)
                    .frame(width: width, height: height)
                    .shadow(radius: 5)

                // 移動掃描線（自上而下）
                Rectangle()
                    .fill(.white.opacity(0.9))
                    .frame(width: width * 0.9, height: 2)
                    // 以 ZStack 中心為原點：從 -height/2（上緣）到 +height/2（下緣）
                    .offset(y: (-height/2 + 1) + progress * (height - 2))
                    .onAppear {
                        progress = 0
                        // 確保第一幀先繪製在最上緣，再開始動畫
                        DispatchQueue.main.async {
                            withAnimation(.linear(duration: 1.2).repeatForever(autoreverses: false)) {
                                progress = 1
                            }
                        }
                    }
            }
            .position(x: geo.size.width/2, y: top + height/2)
        }
        .ignoresSafeArea()
    }
}

private struct CameraPreview: UIViewRepresentable {
    @Binding var torchOn: Bool
    var onFound: (String) -> Void

    // Container view to manage preview layer layout
    final class CameraContainerView: UIView {
        var previewLayer: AVCaptureVideoPreviewLayer?
        override func layoutSubviews() {
            super.layoutSubviews()
            previewLayer?.frame = bounds
        }
    }

    func makeCoordinator() -> Coordinator { Coordinator(onFound: onFound) }

    func makeUIView(context: Context) -> UIView {
        let container = CameraContainerView(frame: .zero)

        let session = context.coordinator.session
        let layer = AVCaptureVideoPreviewLayer(session: session)
        layer.videoGravity = .resizeAspectFill
        // 先給一個初始 frame，避免初次為 0 導致全白
        layer.frame = UIScreen.main.bounds
        // 設定為直向（避免有時為未知導致不顯示）
        layer.connection?.videoOrientation = .portrait

        container.layer.addSublayer(layer)
        container.previewLayer = layer
        context.coordinator.previewLayer = layer

        context.coordinator.configureSession()

        if !session.isRunning {
            session.startRunning()
        }

        return container
    }

    func updateUIView(_ uiView: UIView, context: Context) {
        context.coordinator.setTorch(enabled: torchOn)
    }

    class Coordinator: NSObject, AVCaptureMetadataOutputObjectsDelegate {
        let session = AVCaptureSession()
        var previewLayer: AVCaptureVideoPreviewLayer?
        private let onFound: (String) -> Void

        override init() {
            self.onFound = { _ in }
            super.init()
            NotificationCenter.default.addObserver(self, selector: #selector(handleStop), name: .stopCameraSession, object: nil)
        }

        init(onFound: @escaping (String) -> Void) {
            self.onFound = onFound
            super.init()
            NotificationCenter.default.addObserver(self, selector: #selector(handleStop), name: .stopCameraSession, object: nil)
        }

        deinit {
            NotificationCenter.default.removeObserver(self)
        }

        func configureSession() {
            session.beginConfiguration()
            session.sessionPreset = .high

            guard let device = AVCaptureDevice.default(for: .video),
                  let input = try? AVCaptureDeviceInput(device: device),
                  session.canAddInput(input) else {
                session.commitConfiguration()
                return
            }
            session.addInput(input)

            let output = AVCaptureMetadataOutput()
            guard session.canAddOutput(output) else {
                session.commitConfiguration()
                return
            }
            session.addOutput(output)
            output.setMetadataObjectsDelegate(self, queue: .main)
            output.metadataObjectTypes = [.qr]

            session.commitConfiguration()

            if !self.session.isRunning {
                self.session.startRunning()
            }
        }

        func setTorch(enabled: Bool) {
            guard let device = AVCaptureDevice.default(for: .video), device.hasTorch else { return }
            do {
                try device.lockForConfiguration()
                device.torchMode = enabled ? .on : .off
                device.unlockForConfiguration()
            } catch { }
        }

        func metadataOutput(_ output: AVCaptureMetadataOutput, didOutput metadataObjects: [AVMetadataObject], from connection: AVCaptureConnection) {
            guard let obj = metadataObjects.first as? AVMetadataMachineReadableCodeObject,
                  obj.type == .qr,
                  let value = obj.stringValue, !value.isEmpty else { return }
            // 偵測到即回傳
            onFound(value)
        }

        func stopRunning() {
            if session.isRunning {
                session.stopRunning()
            }
        }

        @objc private func handleStop() {
            stopRunning()
        }
    }
}

// MARK: - 從相簿擷取 QR 內容
private struct QRPhotoPicker: UIViewControllerRepresentable {
    var onFound: (String) -> Void

    func makeUIViewController(context: Context) -> PHPickerViewController {
        var config = PHPickerConfiguration(photoLibrary: .shared())
        config.filter = .images
        config.selectionLimit = 1
        let picker = PHPickerViewController(configuration: config)
        picker.delegate = context.coordinator
        return picker
    }

    func updateUIViewController(_ uiViewController: PHPickerViewController, context: Context) {}

    func makeCoordinator() -> Coordinator { Coordinator(onFound: onFound) }

    class Coordinator: NSObject, PHPickerViewControllerDelegate {
        let onFound: (String) -> Void
        init(onFound: @escaping (String) -> Void) { self.onFound = onFound }

        func picker(_ picker: PHPickerViewController, didFinishPicking results: [PHPickerResult]) {
            guard let provider = results.first?.itemProvider else {
                picker.dismiss(animated: true)
                return
            }
            if provider.canLoadObject(ofClass: UIImage.self) {
                provider.loadObject(ofClass: UIImage.self) { object, _ in
                    DispatchQueue.main.async {
                        picker.dismiss(animated: true)
                    }
                    guard let image = object as? UIImage, let cg = image.cgImage else { return }
                    // 使用 Vision 偵測 QR
                    let request = VNDetectBarcodesRequest { req, _ in
                        if let obs = req.results?.compactMap({ $0 as? VNBarcodeObservation }).first,
                           let payload = obs.payloadStringValue, !payload.isEmpty {
                            DispatchQueue.main.async { self.onFound(payload) }
                        }
                    }
                    request.symbologies = [.QR]
                    let handler = VNImageRequestHandler(cgImage: cg, options: [:])
                    try? handler.perform([request])
                }
            } else {
                picker.dismiss(animated: true)
            }
        }
    }
}

private struct RenameEdgeSheet: View {
    @Environment(\.dismiss) private var dismiss
    @State private var newName: String

    let edge: EdgeSummary
    let onSubmit: (String) -> Void

    init(edge: EdgeSummary, onSubmit: @escaping (String) -> Void) {
        self.edge = edge
        self.onSubmit = onSubmit
        _newName = State(initialValue: edge.displayName)
    }

    private var trimmedName: String { newName.trimmed }
    private var isValid: Bool { !trimmedName.isEmpty && trimmedName.count <= 16 }

    var body: some View {
        NavigationStack {
            Form {
                Section("重新命名 Edge") {
                    TextField("顯示名稱", text: $newName)
                        .onChangeCompat(of: newName) { _, value in
                            if value.count > 16 {
                                newName = String(value.prefix(16))
                            }
                        }
                }
                if !isValid {
                    Text("名稱需為 1-16 字元")
                        .font(.footnote)
                        .foregroundColor(.red)
                }
            }
            .navigationTitle("重新命名")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("取消") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("儲存") {
                        onSubmit(trimmedName)
                        dismiss()
                    }
                    .disabled(!isValid)
                }
            }
        }
    }
}

private struct EdgePasswordSheet: View {
    @Environment(\.dismiss) private var dismiss
    @State private var currentPassword = ""
    @State private var newPassword = ""
    @State private var confirmPassword = ""

    let edge: EdgeSummary
    let onSubmit: (String, String) -> Void

    private var trimmedCurrent: String { currentPassword.trimmed }
    private var trimmedNew: String { newPassword.trimmed }
    private var trimmedConfirm: String { confirmPassword.trimmed }
    private var isValid: Bool { !trimmedCurrent.isEmpty && !trimmedNew.isEmpty && trimmedNew == trimmedConfirm }

    var body: some View {
        NavigationStack {
            Form {
                Section("更新 Edge 密碼") {
                    SecureField("目前密碼", text: $currentPassword)
                    SecureField("新密碼", text: $newPassword)
                    SecureField("確認新密碼", text: $confirmPassword)
                }
                if trimmedNew != trimmedConfirm {
                    Text("兩次輸入的新密碼不同")
                        .font(.footnote)
                        .foregroundColor(.red)
                }
            }
            .navigationTitle(edge.displayName.isEmpty ? edge.edgeId : edge.displayName)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("取消") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("更新") {
                        onSubmit(trimmedCurrent, trimmedNew)
                        dismiss()
                    }
                    .disabled(!isValid)
                }
            }
        }
    }
}

// MARK: - Backward-compatible onChange helper

private extension View {
    @ViewBuilder
    func onChangeCompat<Value: Equatable>(
        of value: Value,
        perform action: @escaping (_ oldValue: Value, _ newValue: Value) -> Void
    ) -> some View {
        if #available(iOS 17.0, *) {
            self.onChange(of: value) { oldValue, newValue in
                action(oldValue, newValue)
            }
        } else {
            // Fallback for iOS 16 及更早版本
            self.onChange(of: value) { newValue in
                action(newValue, newValue)
            }
        }
    }
}


// Notification extension for stopCameraSession
extension Notification.Name {
    static let stopCameraSession = Notification.Name("stopCameraSession")
}

// MARK: - Liquid Button (iOS 26-style) using ButtonStyle for reliable taps
private struct LiquidIconButtonStyle: ButtonStyle {
    let systemName: String
    var isActive: Bool = false

    func makeBody(configuration: Configuration) -> some View {
        LiquidIconVisual(systemName: systemName, isActive: isActive, isPressed: configuration.isPressed)
            .contentShape(Circle())
            .accessibilityLabel(Text(systemName))
    }
}

private struct LiquidIconVisual: View {
    let systemName: String
    var isActive: Bool
    var isPressed: Bool

    var body: some View {
        let baseSize: CGFloat = 68
        ZStack {
            // 背板：厚實液態玻璃 + 主題色輕染
            Circle()
                .fill(.ultraThinMaterial)
                .frame(width: baseSize, height: baseSize)
                .overlay(
                    Circle()
                        .fill(Color.accentColor.opacity(isActive ? 0.16 : 0.12))
                        .blur(radius: 12)
                )
                // 靜態高光（更穩定，不干擾點擊）
                .overlay(
                    Circle()
                        .fill(
                            RadialGradient(
                                colors: [Color.white.opacity(0.85), Color.white.opacity(0.0)],
                                center: .topLeading,
                                startRadius: 6,
                                endRadius: baseSize
                            )
                        )
                        .blur(radius: 1.0)
                )
                .overlay(
                    // 邊緣高光 + 內陰影
                    Circle().strokeBorder(
                        LinearGradient(
                            colors: [Color.white.opacity(0.9), Color.white.opacity(0.28)],
                            startPoint: .topLeading,
                            endPoint: .bottomTrailing
                        ), lineWidth: 1.2
                    )
                )
                .overlay(
                    Circle()
                        .stroke(Color.black.opacity(0.10), lineWidth: 0.6)
                        .blur(radius: 1.2)
                        .mask(
                            Circle().fill(
                                LinearGradient(colors: [.black, .clear], startPoint: .top, endPoint: .bottom)
                            )
                        )
                )
                .shadow(color: Color.black.opacity(0.18), radius: 10, x: 0, y: 6)
                .shadow(color: Color.white.opacity(0.25), radius: 1, x: 0, y: 1)
                .overlay(
                    // 細緻雜訊，讓材質更像玻璃
                    Circle()
                        .fill(
                            LinearGradient(colors: [Color.white.opacity(0.04), Color.black.opacity(0.03)], startPoint: .top, endPoint: .bottom)
                        )
                        .blendMode(.softLight)
                )

            // Icon
            Image(systemName: systemName)
                .font(.system(size: 26, weight: .semibold))
                .foregroundStyle(isActive ? Color.white : Color.primary)
                .shadow(color: isActive ? Color.accentColor.opacity(0.6) : .clear, radius: 8)

            // 主題色 Glow（啟用時）
            Circle()
                .stroke(isActive ? Color.accentColor : Color.clear, lineWidth: 3)
                .opacity(isActive ? 0.7 : 0.0)
                .blur(radius: 3)
                .frame(width: baseSize, height: baseSize)

            // 高光掃掠：用 isPressed 切換角度即可（不需自訂手勢）
            Circle()
                .stroke(
                    AngularGradient(
                        gradient: Gradient(colors: [
                            Color.white.opacity(0.0),
                            Color.white.opacity(0.35),
                            Color.white.opacity(0.0)
                        ]),
                        center: .center
                    ), lineWidth: 2
                )
                .rotationEffect(.degrees(isPressed ? 18 : -18))
                .opacity(0.9)
                .frame(width: baseSize - 2, height: baseSize - 2)
        }
        .frame(width: baseSize, height: baseSize)
        .scaleEffect(isPressed ? 0.92 : (isActive ? 1.05 : 1.0))
        .animation(.spring(response: 0.35, dampingFraction: 0.8), value: isPressed)
        .animation(.spring(response: 0.4, dampingFraction: 0.85), value: isActive)
    }
}
