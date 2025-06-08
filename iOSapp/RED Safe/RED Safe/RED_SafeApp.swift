import SwiftUI

@main
struct RED_SafeApp: App {
    @StateObject private var auth = AuthManager.shared
    @State private var showLaunch = true
    var body: some Scene {
        WindowGroup {
            Group {
                if auth.isLoading || showLaunch {
                    LoadView()
                } else if auth.isLoggedIn {
                    HomeView(home_user_name: auth.userName ?? "")
                } else {
                    SignInView()
                }
            }
            .onAppear {
                DispatchQueue.main.asyncAfter(deadline: .now() + 1.5) {
                    showLaunch = false
                }
                auth.loadSavedSession()
            }
        }
    }
}
