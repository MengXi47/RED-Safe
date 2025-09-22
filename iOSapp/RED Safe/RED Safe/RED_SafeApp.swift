import SwiftUI

@main
struct RED_SafeApp: App {
    @StateObject private var auth = AuthManager.shared

    var body: some Scene {
        WindowGroup {
            RootRouter()
                .environmentObject(auth)
                .onAppear { auth.bootstrap() }
        }
    }
}

private struct RootRouter: View {
    @EnvironmentObject private var auth: AuthManager
    @State private var showSplashOverlay = true

    var body: some View {
        ZStack {
            switch auth.phase {
            case .launching, .refreshing:
                LoadView()
            case .authenticated:
                HomeView()
            case .signedOut:
                SignInView()
            }

            if showSplashOverlay {
                LoadView()
                    .transition(.opacity)
            }
        }
        .onChange(of: auth.phase) { newPhase in
            guard showSplashOverlay else { return }
            if newPhase == .authenticated || newPhase == .signedOut {
                DispatchQueue.main.asyncAfter(deadline: .now() + 1) {
                    withAnimation(.easeOut(duration: 0.35)) {
                        showSplashOverlay = false
                    }
                }
            }
        }
    }
}
