import SwiftUI
import UIKit

struct SignInView: View {
    @StateObject private var auth = AuthManager.shared
    @State private var email: String = ""
    @State private var password: String = ""
    @State private var isPasswordVisible: Bool = false
    @State private var animate: Bool = false
    @State private var emailSubmitted: Bool = false
    @State private var passwordSubmitted: Bool = false
    @State private var errorMessage: String?                // 錯誤訊息
//    @State private var homeDevices: [DeviceStatus] = []     // 傳給 HomeView 的裝置清單
    @State private var showSignInAlert = false              // 顯示登入結果
    @State private var signInMessage = ""                   // Alert 內容
    @State private var proceedHomeAfterAlert = false
    
    // 用來追蹤哪個欄位正在聚焦
    @FocusState private var focusedField: Field?
    
    private enum Field {
        case email, password
    }
    
    var body: some View {
        NavigationStack {
            ZStack {
                // 1. 全螢幕漸層背景
                LinearGradient(
                    gradient: Gradient(colors: [
                        Color(red: 240/255, green: 248/255, blue: 255/255),
                        Color(red: 210/255, green: 235/255, blue: 250/255)
                    ]),
                    startPoint: .topLeading,
                    endPoint: .bottomTrailing
                )
                .ignoresSafeArea()
                .contentShape(Rectangle())
                .onTapGesture {
                    // 點擊空白時隱藏鍵盤並驗證已輸入欄位
                    UIApplication.shared.sendAction(#selector(UIResponder.resignFirstResponder), to: nil, from: nil, for: nil)
                    if !email.isEmpty { emailSubmitted = true }
                    if !password.isEmpty { passwordSubmitted = true }
                }
                
                VStack(spacing: 20) {
                    // 2. Logo
                    Image("RED_Safe_icon1")
                        .resizable()
                        .scaledToFit()
                        .frame(width: 300, height: 300)
                        .padding(.bottom, 10)
                        .scaleEffect(animate ? 1 : 0.5)
                        .opacity(animate ? 1 : 0)
                        .animation(.spring(response: 0.6, dampingFraction: 0.6).delay(0.1), value: animate)
                        .onTapGesture {
                            // 點擊 Logo 隱藏鍵盤並驗證已輸入欄位
                            UIApplication.shared.sendAction(#selector(UIResponder.resignFirstResponder), to: nil, from: nil, for: nil)
                            if !email.isEmpty { emailSubmitted = true }
                            if !password.isEmpty { passwordSubmitted = true }
                        }
                    
                    // 3. Email 輸入框
                    HStack {
                        Image(systemName: "envelope")
                            .foregroundColor(.gray)
                        TextField("Email", text: $email)
                            .autocapitalization(.none)
                            .disableAutocorrection(true)
                            .focused($focusedField, equals: .email)
                    }
                    .offset(x: animate ? 0 : -200)
                    .opacity(animate ? 1 : 0)
                    .animation(.easeOut(duration: 0.5).delay(0.2), value: animate)
                    .padding()
                    .background(Color.white.opacity(0.8))
                    .cornerRadius(10)
                    .shadow(color: Color.black.opacity(0.1), radius: 5, x: 0, y: 2)
                    .submitLabel(.done)
                    .onSubmit { emailSubmitted = true }
                    .overlay(
                        RoundedRectangle(cornerRadius: 10)
                            .stroke((emailSubmitted && email.isEmpty) ? Color.red : Color.clear, lineWidth: 2)
                    )
                    .padding(.horizontal)
                    
                    // 4. Password 輸入框 (顯示/隱藏切換)
                    HStack {
                        Image(systemName: "lock")
                            .padding(.horizontal, 4.5)
                            .foregroundColor(.gray)
                        if isPasswordVisible {
                            TextField("Password", text: $password)
                                .focused($focusedField, equals: .password)
                        } else {
                            SecureField("Password", text: $password)
                                .focused($focusedField, equals: .password)
                        }
                        Button(action: {
                            isPasswordVisible.toggle()
                        }) {
                            Image(systemName: isPasswordVisible ? "eye.slash" : "eye")
                                .foregroundColor(.gray)
                        }
                    }
                    .offset(x: animate ? 0 : 200)
                    .opacity(animate ? 1 : 0)
                    .animation(.easeOut(duration: 0.5).delay(0.3), value: animate)
                    .padding()
                    .background(Color.white.opacity(0.8))
                    .cornerRadius(10)
                    .shadow(color: Color.black.opacity(0.1), radius: 5, x: 0, y: 2)
                    .submitLabel(.done)
                    .onSubmit { passwordSubmitted = true }
                    .overlay(
                        RoundedRectangle(cornerRadius: 10)
                            .stroke((passwordSubmitted && password.isEmpty) ? Color.red : Color.clear, lineWidth: 2)
                    )
                    .padding(.horizontal)
                    
                    // 5. Sign In 按鈕
                    Button(action: {
                        signIn()
                    }) {
                        Text("登入")
                            .font(.headline)
                            .foregroundColor(.white)
                            .frame(maxWidth: .infinity)
                            .padding()
                    }
                    .background(
                        LinearGradient(
                            gradient: Gradient(colors: email.isEmpty || password.isEmpty
                                               ? [Color.gray.opacity(0.6), Color.gray.opacity(0.3)]
                                               : [Color(red: 65/255, green: 160/255, blue: 255/255),
                                                  Color(red: 120/255, green: 190/255, blue: 255/255)]),
                            startPoint: .leading,
                            endPoint: .trailing
                        )
                    )
                    .clipShape(RoundedRectangle(cornerRadius: 10))
                    .offset(y: animate ? 0 : 50)
                    .opacity(animate ? 1 : 0)
                    .animation(.easeOut(duration: 0.5).delay(0.4), value: animate)
                    .padding(.horizontal)
                    .disabled(email.isEmpty || password.isEmpty)
                    .shadow(color: Color.black.opacity(0.2), radius: 5, x: 0, y: 2)
                    
                    // 6. Sign Up 按鈕 (NavigationLink)
                    NavigationLink(
                        destination: SignUpView()
                    ) {
                        Text("註冊")
                            .font(.headline)
                            .foregroundColor(.blue)
                            .frame(maxWidth: .infinity)
                            .padding()
                    }
                    .offset(y: animate ? 0 : 50)
                    .opacity(animate ? 1 : 0)
                    .animation(.easeOut(duration: 0.5).delay(0.5), value: animate)
                    .cornerRadius(10)
                    .padding(.top, -10)          // 讓「註冊」貼近「登入」
                }
                .onAppear() {
                    animate = true
                }
            }
            
            // 彈出訊息，1 秒後自動消失
            .overlay(
                Group{
                    if showSignInAlert {
                        Text(signInMessage)
                            .padding()
                            .background(Color.black.opacity(0.7))
                            .foregroundStyle(.white)
                            .cornerRadius(8)
                            .transition(.opacity)
                            .onAppear {
                                DispatchQueue.main.asyncAfter(deadline: .now() + 1) {
                                    showSignInAlert = false
                                    if proceedHomeAfterAlert {
                                        auth.setLoggedIn(true)

                                    }
                                }
                            }
                    }
                }
            )
            
            // 當焦點變更後，若不再聚焦在Email或Password就觸發驗證
            .onChange(of: focusedField) {
                // 當焦點變更後，若不再聚焦在Email或Password就觸發驗證
                if focusedField != .email && !email.isEmpty {
                    emailSubmitted = true
                }
                if focusedField != .password && !password.isEmpty {
                    passwordSubmitted = true
                }
            }
            
            .onDisappear {
                // 重置所有狀態
                email = ""
                password = ""
                isPasswordVisible = false
                emailSubmitted = false
                passwordSubmitted = false
                errorMessage = nil
                showSignInAlert = false
                signInMessage = ""
                proceedHomeAfterAlert = false
            }
        }
    }
    
    private func signIn() {
        signInMessage   = ""
        errorMessage    = nil
        auth.signIn(email: email, password: password) { result in
            DispatchQueue.main.async {
                switch result {
                case .success(let response):
                    signInMessage = response.error_code.localizedDescription
                    if response.error_code == .success {
                        proceedHomeAfterAlert = true
                    }
                    showSignInAlert = true
                case .failure(let error):
                    switch error {
                    case .invalidURL:
                        print("無效的伺服器位址")
                    case .serverError(let status):
                        print("伺服器錯誤 (\(status))")
                    case .decodingError:
                        print("資料解析失敗")
                    case .unknown(let err):
                        print("發生錯誤：\(err.localizedDescription)")
                    }
                    signInMessage = "未知錯誤"
                    showSignInAlert = true
                }
            }
        }
    }
}

#Preview {
    SignInView()
}
