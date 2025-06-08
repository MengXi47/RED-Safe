import Foundation
import SwiftUI
import UIKit

struct SignUpView: View {
    @Environment(\.dismiss) private var dismiss
    @State private var email: String = ""
    @State private var username: String = ""
    @State private var password: String = ""

    @State private var emailSubmitted: Bool = false
    @State private var usernameSubmitted: Bool = false
    @State private var passwordSubmitted: Bool = false
    
    @State private var errorMessage: String?
    @State private var isLoading: Bool = false
    
    // 用來控制箭頭透明度
    @State private var popProgress: CGFloat = 0

    var isEmailValid: Bool {
        let emailRegEx = "[A-Z0-9a-z._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,64}"
        return NSPredicate(format: "SELF MATCHES %@", emailRegEx).evaluate(with: email)
    }
    var isPasswordValid: Bool {
        let passwordRegEx = "^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d!@#$%^&*()_+=-]{8,}$"
        return NSPredicate(format: "SELF MATCHES %@", passwordRegEx).evaluate(with: password)
    }
    var isFormValid: Bool {
        isEmailValid && isPasswordValid
    }

    @State private var animate: Bool = false
    @State private var isPasswordVisible: Bool = false
    @State private var showResponseAlert = false
    @State private var responseMessage = ""
    @State private var shouldDismiss = false   // Alert 按下確定後是否返回登入

    var body: some View {
        ZStack {
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
                // 點擊空白時隱藏鍵盤並觸發所有欄位驗證
                UIApplication.shared.sendAction(#selector(UIResponder.resignFirstResponder), to: nil, from: nil, for: nil)
                if !email.isEmpty { emailSubmitted = true }
                if !username.isEmpty { usernameSubmitted = true }
                if !password.isEmpty { passwordSubmitted = true }
            }
            VStack(spacing: 20) {
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
                        if !username.isEmpty { usernameSubmitted = true }
                        if !password.isEmpty { passwordSubmitted = true }
                    }

                // Email 輸入框
                HStack {
                    Image(systemName: "envelope")
                        .foregroundColor(.gray)
                    TextField("Email", text: $email)
                        .autocapitalization(.none)
                        .disableAutocorrection(true)
                }
                .offset(x: animate ? 0 : -200)
                .opacity(animate ? 1 : 0)
                .animation(.easeOut(duration: 0.5).delay(0.2), value: animate)
                .padding()
                .background(Color.white.opacity(0.9))
                .submitLabel(.done)
                .onSubmit {
                    emailSubmitted = true
                }
                .overlay(
                    RoundedRectangle(cornerRadius: 10)
                        .stroke((emailSubmitted && !email.isEmpty && !isEmailValid) ? Color.red : Color.clear, lineWidth: 2)
                )
                .cornerRadius(10)
                .shadow(color: Color.black.opacity(0.1), radius: 5, x: 0, y: 2)
                .padding(.horizontal)

                // Username 輸入框
                HStack {
                    Image(systemName: "person")
                        .foregroundColor(.gray)
                        .padding(.horizontal, 3)
                    TextField("Username", text: $username)
                        .autocapitalization(.none)
                        .disableAutocorrection(true)
                }
                .offset(x: animate ? 0 : 200)
                .opacity(animate ? 1 : 0)
                .animation(.easeOut(duration: 0.5).delay(0.3), value: animate)
                .padding()
                .background(Color.white.opacity(0.9))
                .submitLabel(.done)
                .onSubmit {
                    usernameSubmitted = true
                }
                .overlay(
                    RoundedRectangle(cornerRadius: 10)
                        .stroke((usernameSubmitted && !username.isEmpty) ? Color.red : Color.clear, lineWidth: 2)
                )
                .cornerRadius(10)
                .shadow(color: Color.black.opacity(0.1), radius: 5, x: 0, y: 2)
                .padding(.horizontal)

                // Password 輸入框
                HStack {
                    Image(systemName: "lock")
                        .padding(.horizontal, 4.5)
                        .foregroundColor(.gray)
                    if isPasswordVisible {
                        TextField("Password", text: $password)
                    } else {
                        SecureField("Password", text: $password)
                    }
                    Button(action: {
                        isPasswordVisible.toggle()
                    }) {
                        Image(systemName: isPasswordVisible ? "eye.slash" : "eye")
                            .foregroundColor(.gray)
                    }
                }
                .offset(x: animate ? 0 : -200)
                .opacity(animate ? 1 : 0)
                .animation(.easeOut(duration: 0.5).delay(0.4), value: animate)
                .padding()
                .background(Color.white.opacity(0.9))
                .submitLabel(.done)
                .onSubmit {
                    passwordSubmitted = true
                }
                .overlay(
                    RoundedRectangle(cornerRadius: 10)
                        .stroke((passwordSubmitted && !password.isEmpty && !isPasswordValid) ? Color.red : Color.clear, lineWidth: 2)
                )
                .cornerRadius(10)
                .shadow(color: Color.black.opacity(0.1), radius: 5, x: 0, y: 2)
                .padding(.horizontal)

                // Submit 按鈕
                Button(action: {
                    signUp()
                }) {
                    Text("註冊")
                        .font(.headline)
                        .foregroundColor(.white)
                        .frame(maxWidth: .infinity)
                        .padding()
                }
                .background(
                    LinearGradient(
                        gradient: Gradient(colors: !isFormValid
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
                .disabled(email.isEmpty || username.isEmpty || password.isEmpty)
                .shadow(color: Color.black.opacity(0.2), radius: 5, x: 0, y: 2)
            }
            
            .onAppear {
                animate = true
            }
            .onDisappear(){
                email = ""
                username = ""
                password = ""
                errorMessage = nil
            }
        }
        // 註冊結果提示
        .overlay(
            Group {
                if showResponseAlert {
                    Text(responseMessage)
                        .padding()
                        .background(Color.black.opacity(0.7))
                        .foregroundColor(.white)
                        .cornerRadius(8)
                        .transition(.opacity)
                        .onAppear {
                            DispatchQueue.main.asyncAfter(deadline: .now() + 2) {
                                showResponseAlert = false
                                if shouldDismiss {
                                    dismiss()
                                }
                            }
                        }
                }
            }
        )
        .toolbar {
            ToolbarItem(placement: .navigationBarLeading) {
                Image(systemName: "chevron.left")
                    .font(.title2.weight(.semibold))
                    .opacity(1 - popProgress)        // 隨進度淡出：滑一半時透明度 0.5
                    .onTapGesture { dismiss() }        // 點擊仍可返回
            }
        }
        .navigationBarBackButtonHidden(true)           // 只藏系統箭頭
    }
    
    private func signUp() {
        // 先重置錯誤及顯示 loading
        errorMessage = nil
        Network.shared.signUp(email: email, userName: username, password: password) { result in
            DispatchQueue.main.async {
                isLoading = false
                switch result {
                case .success(let response):
                    responseMessage = "\(response.error_code.localizedDescription)"
                    // 若為成功則設定 shouldDismiss = true，否則 false
                    shouldDismiss = (response.error_code == .success)
                    if response.error_code == .success, let name = response.user_name {
                        UserDefaults.standard.set(name, forKey: AuthManager.shared.nameKey)
                    }
                    showResponseAlert = true
                case .failure(let error):
                    // 失敗：顯示錯誤訊息
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
                    responseMessage = "網路錯誤"
                    shouldDismiss = false
                    showResponseAlert = true
                }
            }
        }
    }
}

#Preview {
    SignUpView()
}
