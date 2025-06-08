import SwiftUI

@main
struct RED_SafeApp: App {
    @StateObject private var auth = AuthManager.shared
    var body: some Scene {
        WindowGroup {
            Group {
                if auth.isLoading {
                    LoadView()
                } else if auth.isLoggedIn {
                    HomeView(userName: auth.userName ?? "", devices: nil)
                } else {
                    SignInView()
                }
            }
            .onAppear {
                auth.loadSavedSession()
            }
        }
    }
}
