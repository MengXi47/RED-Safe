//
//  SignInView.swift
//  RED Safe
//
//  Created by boen on 2025/5/26.
//

import SwiftUI
import UIKit

struct SignInView: View {
    @State private var email: String = ""
    @State private var password: String = ""
    @State private var isPasswordVisible: Bool = false
    @State private var animate: Bool = false

    @State private var emailSubmitted: Bool = false
    @State private var passwordSubmitted: Bool = false
    
    @State private var isLoading = false
    @State private var errorMessage: String?
    @State private var navigateHome = false               // 觸發跳轉
    @State private var homeUserName = ""                  // 傳給 HomeView 的使用者名稱
    @State private var homeDevices: [DeviceStatus] = []   // 傳給 HomeView 的裝置清單
    @State private var showSignInAlert = false         // 顯示登入結果
    @State private var signInMessage = ""              // Alert 內容
    @State private var proceedHomeAfterAlert = false   // 點確定後是否跳轉

    var isEmailValid: Bool {
        let emailRegEx = "[A-Z0-9a-z._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}"
        return NSPredicate(format: "SELF MATCHES %@", emailRegEx).evaluate(with: email)
    }
    var isPasswordValid: Bool {
        let passwordRegEx = "^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d!@#$%^&*()_+=-]{8,}$"
        return NSPredicate(format: "SELF MATCHES %@", passwordRegEx).evaluate(with: password)
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

                    // 3. Email 輸入框
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
                    .background(Color.white.opacity(0.8))
                    .cornerRadius(10)
                    .shadow(color: Color.black.opacity(0.1), radius: 5, x: 0, y: 2)
                    .submitLabel(.done)
                    .onSubmit { emailSubmitted = true }
                    .overlay(
                        RoundedRectangle(cornerRadius: 10)
                            .stroke((emailSubmitted && !isEmailValid) ? Color.red : Color.clear, lineWidth: 2)
                    )
                    .padding(.horizontal)

                    // 4. Password 輸入框 (顯示/隱藏切換)
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
                            .stroke((passwordSubmitted && !isPasswordValid) ? Color.red : Color.clear, lineWidth: 2)
                    )
                    .padding(.horizontal)

                    // 5. Sign In 按鈕
                    Button(action: {
                        signIn()
                    }) {
                        Text("Sign In")
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
                        Text("Sign Up")
                            .font(.headline)
                            .foregroundColor(.white)
                            .frame(maxWidth: .infinity)
                            .padding()
                    }
                    .background(Color.clear)
                    .overlay(
                        RoundedRectangle(cornerRadius: 10)
                            .stroke(Color(red: 65/255, green: 160/255, blue: 255/255), lineWidth: 2)
                    )
                    .offset(y: animate ? 0 : 50)
                    .opacity(animate ? 1 : 0)
                    .animation(.easeOut(duration: 0.5).delay(0.5), value: animate)
                    .cornerRadius(10)
                    .padding(.horizontal)
                }
                .onAppear {
                    animate = true
                }
            }
            // 登入結果 Alert
            .alert("登入結果", isPresented: $showSignInAlert) {
                Button("確定") {
                    if proceedHomeAfterAlert {
                        navigateHome = true
                    }
                }
                    } message: {
                Text(signInMessage)
            }
            // 跳轉到 HomeView
            .navigationDestination(isPresented: $navigateHome) {
                HomeView(userName: homeUserName, devices: homeDevices)
            }
        }
    }
    
    private func signIn() {
        // 先重置錯誤及顯示 loading
        errorMessage = nil
        isLoading = true
        Network.shared.signIn(email: email, password: password) { result in
            DispatchQueue.main.async {
                isLoading = false
                switch result {
                case .success(let response):
                    // 顯示伺服器回傳訊息
                    signInMessage = response.error_code.localizedDescription
                    if response.error_code == .success {
                        // 成功時預先設定資料，待使用者按確定後跳轉
                        homeUserName = response.user_name ?? "Unknown User"
                        homeDevices = [
                            DeviceStatus(serial: "1234-5678-ABCD", isOnline: true),
                            DeviceStatus(serial: "9876-5432-ZYXW", isOnline: false)
                        ]
                        proceedHomeAfterAlert = true
                    } else {
                        proceedHomeAfterAlert = false
                    }
                    showSignInAlert = true
                case .failure(let error):
                    // 失敗：顯示錯誤訊息
                    switch error {
                    case .invalidURL:
                        errorMessage = "無效的伺服器位址"
                    case .serverError(let status):
                        errorMessage = "伺服器錯誤 (\(status))"
                    case .decodingError:
                        errorMessage = "資料解析失敗"
                    case .unknown(let err):
                        errorMessage = "發生錯誤：\(err.localizedDescription)"
                    }
                }
            }
        }
    }
}

#Preview {
    SignInView()
}
