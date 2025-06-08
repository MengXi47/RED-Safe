import SwiftUI

@main
struct RED_SafeApp: App {
    @StateObject private var auth = AuthManager.shared
    @State private var showLaunch = true
    @Environment(\.scenePhase) private var scenePhase
    var body: some Scene {
        WindowGroup {
            Group {
                if auth.isLoading || showLaunch {
                    LoadView()
                } else if auth.isLoggedIn {
                    HomeView(userName: auth.userName ?? "", devices: nil)
                } else {
                    SignInView()
                }
            }
            .onAppear {
                DispatchQueue.main.asyncAfter(deadline: .now() + 2) {
                    showLaunch = false
                }
                auth.loadSavedSession()
            }
            .onChange(of: scenePhase) { newPhase in
                if newPhase == .background {
                    auth.clearAccessToken()
                }
            }
        }
    }
}
