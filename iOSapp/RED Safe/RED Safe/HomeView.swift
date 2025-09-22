import SwiftUI
import UIKit

/// HomeView 為登入後的主要容器，負責整合儀表板與帳號兩大分頁。
struct HomeView: View {
    @StateObject private var homeVM = HomeViewModel()
    @StateObject private var profileVM = ProfileViewModel()
    @ObservedObject private var auth = AuthManager.shared

    @State private var deviceSheet: DeviceSheet?
    @State private var deviceToUnbind: EdgeSummary?
    @State private var showUnbindConfirmation = false
    @State private var animateDashboard = false
    @State private var profileSheet: ProfileSheet?

    private var toast: ToastPayload? {
        if homeVM.showMessage, let message = homeVM.message {
            return ToastPayload(text: message, kind: .info)
        }
        if profileVM.showMessage, let message = profileVM.message {
            return ToastPayload(text: message, kind: .info)
        }
        return nil
    }

    private var isBusy: Bool { homeVM.isLoading || profileVM.isWorking }

    var body: some View {
        TabView {
            DashboardView(
                homeVM: homeVM,
                auth: auth,
                deviceSheet: $deviceSheet,
                deviceToUnbind: $deviceToUnbind,
                showUnbindConfirm: $showUnbindConfirmation,
                animateBackground: $animateDashboard
            )
            .tabItem { Label("儀表板", systemImage: "house.fill") }

            AccountView(
                auth: auth,
                profileVM: profileVM,
                profileSheet: $profileSheet
            )
            .tabItem { Label("帳號", systemImage: "person.crop.circle") }
        }
        .task {
            if homeVM.edges.isEmpty {
                homeVM.loadEdges()
            }
        }
        .overlay(alignment: .top) {
            if let toast {
                ToastOverlay(payload: toast)
                    .padding(.top, 18)
                    .transition(.move(edge: .top).combined(with: .opacity))
            }
        }
        .overlay {
            if isBusy {
                ZStack {
                    Color.black.opacity(0.25).ignoresSafeArea()
                    ProgressView()
                        .progressViewStyle(.circular)
                        .tint(.white)
                        .scaleEffect(1.2)
                }
            }
        }
        .animation(.easeInOut(duration: 0.25), value: toast)
    }
}

// MARK: - Toast

private struct ToastPayload: Equatable {
    enum Kind { case info }
    let text: String
    let kind: Kind
}

private struct ToastOverlay: View {
    let payload: ToastPayload

    var body: some View {
        Text(payload.text)
            .font(.subheadline.weight(.medium))
            .foregroundColor(.primary)
            .padding(.horizontal, 18)
            .padding(.vertical, 12)
            .background(.thinMaterial, in: Capsule())
            .shadow(color: Color.black.opacity(0.2), radius: 16, x: 0, y: 12)
    }
}

#Preview {
    HomeView()
}
