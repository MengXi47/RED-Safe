//
//  HomeView.swift
//  RED Safe
//
//  Created by boen on 2025/5/27.
//

import SwiftUI

/// 表示單一裝置（以序號區分）及其線上狀態
struct DeviceStatus: Identifiable {
    let id = UUID()
    let serial: String
    let isOnline: Bool
}

struct HomeView: View {
    let userName: String
    let devices: [DeviceStatus]?

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
                            Text("\(userName)")
                                .font(.title2.bold())
                                .foregroundColor(.primary)
                            Image(systemName: "chevron.down")
                                .foregroundColor(.primary)
                        }
                        Spacer()
                        HStack(spacing: 20) {
                            Image(systemName: "plus")
                            Image(systemName: "bell")
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
                            ForEach(devices ?? []) { device in
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
    }
}

#Preview {
    HomeView(
        userName: "BoEn",
        devices: [
            DeviceStatus(serial: "1234-5678-ABCD", isOnline: true),
            DeviceStatus(serial: "1234-5678-ABCD", isOnline: true),
            DeviceStatus(serial: "1234-5678-ABCD", isOnline: true),
            DeviceStatus(serial: "1234-5678-ABCD", isOnline: true),
            DeviceStatus(serial: "1234-5678-ABCD", isOnline: true),
            DeviceStatus(serial: "1234-5678-ABCD", isOnline: true),
            DeviceStatus(serial: "1234-5678-ABCD", isOnline: true),
            DeviceStatus(serial: "1234-5678-ABCD", isOnline: true),
            DeviceStatus(serial: "1234-5678-ABCD", isOnline: true),
            DeviceStatus(serial: "1234-5678-ABCD", isOnline: true),
            DeviceStatus(serial: "9876-5432-ZYXW", isOnline: false)
        ]
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
