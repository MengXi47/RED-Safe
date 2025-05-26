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

    var isEmailValid: Bool {
        let emailRegEx = "[A-Z0-9a-z._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,64}"
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
                        Color(red: 10/255, green: 60/255, blue: 110/255),
                        Color(red: 10/255, green: 30/255, blue: 60/255)
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
                    .background(Color.white.opacity(0.9))
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
                    .background(Color.white.opacity(0.9))
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
                        // 處理登入動作
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
                                               : [Color(red: 40/255, green: 120/255, blue: 200/255),
                                                  Color(red: 20/255, green: 80/255, blue: 160/255)]),
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
                            .stroke(Color.white, lineWidth: 2)
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
        }
    }
}

#Preview {
    SignInView()
}
