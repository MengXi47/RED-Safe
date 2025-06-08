import SwiftUI

/// 表示單一裝置（以序號區分）及其線上狀態
struct DeviceStatus: Identifiable {
    let id = UUID()
    let serial: String
    let isOnline: Bool
}

struct HomeView: View {
    let home_user_name: String
    @State private var home_devices: [DeviceStatus]?

    var body: some View {
        NavigationStack {
            ZStack {
                // 背景漸層
                LinearGradient(
                    gradient: Gradient(colors: [
                        Color(red: 240/255, green: 248/255, blue: 255/255),
                        Color(red: 210/255, green: 235/255, blue: 250/255)
                    ]),
                    startPoint: .topLeading,
                    endPoint: .bottomTrailing
                )
                .ignoresSafeArea()
                
                VStack(alignment: .leading, spacing: 24) {
                    // 頂部列：左上使用者名稱 + 下拉箭頭 / 右側功能圖示
                    HStack {
                        HStack(spacing: 4) {
                            Text("\(home_user_name)")
                                .font(.title2.bold())
                                .foregroundColor(.primary)
                            Image(systemName: "chevron.down")
                                .foregroundColor(.primary)
                        }
                        Spacer()
                        HStack(spacing: 20) {
                            Image(systemName: "plus")
                            Button {
                                AuthManager.shared.signOut()
                            } label: {
                                Image(systemName: "bell")
                            }
                            .buttonStyle(.plain)
                            Image(systemName: "ellipsis")
                        }
                        .font(.title3)
                        .foregroundColor(.primary)
                    }
                    .padding(.horizontal)
                    .padding(.top, 8)
                    
                    // 中間卡片區：列出每個序號及其狀態
                    ScrollView {
                        VStack(spacing: 16) {
                            ForEach(home_devices ?? []) { device in
                                HStack(spacing: 16) {
                                    Image(systemName: device.isOnline ? "wifi" : "wifi.slash")
                                        .font(.title2)
                                        .foregroundColor(device.isOnline ? .green : .gray)
                                    
                                    VStack(alignment: .leading, spacing: 4) {
                                        Text(device.serial)
                                            .font(.headline)
                                        Text(device.isOnline ? "在線上" : "離線")
                                            .font(.subheadline)
                                            .foregroundColor(device.isOnline ? .green : .red)
                                    }
                                    Spacer()
                                }
                                .padding()
                                .frame(maxWidth: .infinity)
                                .background(Color.white.opacity(0.9))
                                .cornerRadius(12)
                                .shadow(color: Color.black.opacity(0.05), radius: 4, x: 0, y: 2)
                                .padding(.horizontal)
                            }
                        }
                        .padding(.top, 8)
                    }
                    Spacer()
                }
            }
        }
        .navigationBarBackButtonHidden(true)
        .disablePopGesture()
        
        .onAppear {
            Network.shared.getUserInfo { infoResult in
                if case .success(let info) = infoResult, info.error_code == .success {
                    // 成功：更新使用者名稱與裝置陣列
                    DispatchQueue.main.async {
                        home_devices = info.serial_number?
                            .map { DeviceStatus(serial: $0, isOnline: true) } ?? []
                    }
                } else {
                    // 失敗：保持原本登入回傳的資料，或視情況清空
                    DispatchQueue.main.async {
                        home_devices = []   // 失敗就清空裝置清單
                    }
                }
            }
        }
    }
}

#Preview {
    HomeView(
        home_user_name: "BoEn"
    )
}

// MARK: - Disable Pop Gesture Helper
struct DisablePopGestureDetector: UIViewControllerRepresentable {
    func makeUIViewController(context: Context) -> UIViewController {
        let vc = UIViewController()
        DispatchQueue.main.async {
            vc.navigationController?.interactivePopGestureRecognizer?.isEnabled = false
        }
        return vc
    }
    func updateUIViewController(_ uiViewController: UIViewController, context: Context) {}
}

struct DisablePopGestureModifier: ViewModifier {
    func body(content: Content) -> some View {
        content.background(DisablePopGestureDetector())
    }
}

extension View {
    /// 禁用互動式 Pop 手勢
    func disablePopGesture() -> some View {
        self.modifier(DisablePopGestureModifier())
    }
}
