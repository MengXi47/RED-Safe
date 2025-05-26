//
//  SignUpView.swift
//  RED Safe
//
//  Created by boen on 2025/5/26.
//

import Foundation
import SwiftUI
import UIKit

struct SignUpView: View {
    @State private var email: String = ""
    @State private var username: String = ""
    @State private var password: String = ""

    @State private var emailSubmitted: Bool = false
    @State private var usernameSubmitted: Bool = false
    @State private var passwordSubmitted: Bool = false

    var isEmailValid: Bool {
        let emailRegEx = "[A-Z0-9a-z._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,64}"
        return NSPredicate(format: "SELF MATCHES %@", emailRegEx).evaluate(with: email)
    }
    var isUsernameValid: Bool {
        let usernameRegEx = "^[A-Za-z0-9\\u4E00-\\u9FFF\\-_\\.]{1,16}$"
        return NSPredicate(format: "SELF MATCHES %@", usernameRegEx).evaluate(with: username)
    }
    var isPasswordValid: Bool {
        let passwordRegEx = "^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d!@#$%^&*()_+=-]{8,}$"
        return NSPredicate(format: "SELF MATCHES %@", passwordRegEx).evaluate(with: password)
    }
    var isFormValid: Bool {
        isEmailValid && isUsernameValid && isPasswordValid
    }

    @State private var animate: Bool = false
    @State private var isPasswordVisible: Bool = false

    var body: some View {
        ZStack {
            LinearGradient(
                gradient: Gradient(colors: [
                    Color(red: 10/255, green: 60/255, blue: 110/255),
                    Color(red: 10/255, green: 30/255, blue: 60/255)
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
                        .stroke((usernameSubmitted && !username.isEmpty && !isUsernameValid) ? Color.red : Color.clear, lineWidth: 2)
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
                    // TODO: 提交註冊
                }) {
                    Text("Submit")
                        .font(.headline)
                        .foregroundColor(.white)
                        .frame(maxWidth: .infinity)
                        .padding()
                }
                .background(
                    LinearGradient(
                        gradient: Gradient(colors: isFormValid
                                           ? [Color(red: 40/255, green: 120/255, blue: 200/255),
                                              Color(red: 20/255, green: 80/255, blue: 160/255)]
                                           : [Color.gray.opacity(0.6), Color.gray.opacity(0.3)]),
                        startPoint: .leading,
                        endPoint: .trailing
                    )
                )
                .clipShape(RoundedRectangle(cornerRadius: 10))
                .offset(y: animate ? 0 : 50)
                .opacity(animate ? 1 : 0)
                .animation(.easeOut(duration: 0.5).delay(0.5), value: animate)
                .padding(.horizontal)
                .disabled(!isFormValid)
                .shadow(color: Color.black.opacity(0.2), radius: 5, x: 0, y: 2)
            }
            .onAppear {
                animate = true
            }
        }
    }
}

#Preview {
    SignUpView()
}
