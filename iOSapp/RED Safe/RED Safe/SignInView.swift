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
    
    @State private var isLoading = false        // 網路請求中
    @State private var errorMessage: String?    // 錯誤訊息
    
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
    var isFormValid: Bool {
        isEmailValid && isPasswordValid
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
                        Text("登入")
                            .font(.headline)
                            .foregroundColor(.white)
                            .frame(maxWidth: .infinity)
                            .padding()
                    }
                    .background(
                        LinearGradient(
                            gradient: Gradient(colors: email.isEmpty || password.isEmpty || !isFormValid
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
                    .disabled(email.isEmpty || password.isEmpty || !isFormValid)
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
                .onAppear(){
                    animate = true
                }
                .onAppear(){
                    // 清除資料
                    email = ""
                    password = ""
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
        signInMessage   = ""
        errorMessage    = nil
        isLoading       = true
        Network.shared.signIn(email: email, password: password) { result in
            DispatchQueue.main.async {
                isLoading = false
                switch result {
                case .success(let response):
                    signInMessage = response.error_code.localizedDescription
                    if response.error_code == .success {
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
                }
            }
        }
    }
}

#Preview {
    SignInView()
}
