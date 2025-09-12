import SwiftUI     // UIKit 不需要可移除

struct SignUpView: View {
    // MARK: - 環境與狀態
    @Environment(\.dismiss) private var dismiss
    
    @State private var email: String = ""
    @State private var username: String = ""
    @State private var password: String = ""
    
    @State private var emailSubmitted   = false
    @State private var usernameSubmitted = false
    @State private var passwordSubmitted = false
    
    @State private var errorMessage: String?
    @State private var isLoading = false
    @State private var animate   = false
    @State private var isPasswordVisible = false
    
    @State private var showResponseAlert = false
    @State private var responseMessage = ""
    @State private var shouldDismiss = false
    
    // MARK: - 驗證規則
    private var isEmailValid: Bool {
        let regex = "[A-Z0-9a-z._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,64}"
        return NSPredicate(format: "SELF MATCHES %@", regex).evaluate(with: email)
    }
    private var isPasswordValid: Bool {
        let regex = "^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d!@#$%^&*()_+=-]{8,}$"
        return NSPredicate(format: "SELF MATCHES %@", regex).evaluate(with: password)
    }
    private var isFormValid: Bool { isEmailValid && isPasswordValid }
    
    // MARK: - 主要畫面
    var body: some View {
        ZStack {
            background
            content
        }
        .onTapGesture { endEditingAndValidate() }
        .toolbar { backButton }
        .onAppear { animate = true }
        .navigationBarBackButtonHidden(true)
    }
    
    // MARK: - 抽離的子 View
    private var background: some View {
        LinearGradient(
            colors: [
                Color(red: 240/255, green: 248/255, blue: 255/255),
                Color(red: 210/255, green: 235/255, blue: 250/255)
            ],
            startPoint: .topLeading,
            endPoint: .bottomTrailing
        )
        .ignoresSafeArea()
        // 彈出訊息，1 秒後自動消失
        .overlay(
            Group{
                if showResponseAlert {
                    Text(responseMessage)
                        .padding()
                        .background(Color.black.opacity(0.7))
                        .foregroundStyle(.white)
                        .cornerRadius(8)
                        .transition(.opacity)
                        .onAppear {
                            DispatchQueue.main.asyncAfter(deadline: .now() + 1) {
                                showResponseAlert = false
                                if shouldDismiss {
                                    dismiss()
                                }
                            }
                        }
                }
            }
        )
    }
    
    private var content: some View {
        VStack(spacing: 20) {
            logo
            emailField
            usernameField
            passwordField
            submitButton
        }
        .padding(.horizontal)
    }
    
    private var logo: some View {
        Image("RED_Safe_icon1")
            .resizable()
            .scaledToFit()
            .frame(width: 300, height: 300)
            .padding(.bottom, 10)
            .scaleEffect(animate ? 1 : 0.5)
            .opacity(animate ? 1 : 0)
            .animation(.spring(response: 0.6, dampingFraction: 0.6).delay(0.1), value: animate)
            .onTapGesture { endEditingAndValidate() }
    }
    
    private var emailField: some View {
        HStack {
            Image(systemName: "envelope").foregroundColor(.gray)
            TextField("Email", text: $email)
                .keyboardType(.emailAddress)
                .autocapitalization(.none)
                .submitLabel(.done)
                .onSubmit { emailSubmitted = true }
        }
        .animatedField(offset: -200, delay: 0.2, animate: animate)
        .fieldStyle(borderRed: emailSubmitted && !email.isEmpty && !isEmailValid)
    }
    
    private var usernameField: some View {
        HStack {
            Image(systemName: "person").foregroundColor(.gray)
            TextField("Username", text: $username)
                .autocapitalization(.none)
                .submitLabel(.done)
                .submitLabel(.done)
                .onSubmit { usernameSubmitted = true }
        }
        .animatedField(offset: 200, delay: 0.3, animate: animate)
        .fieldStyle(borderRed: usernameSubmitted && username.isEmpty)
    }
    
    private var passwordField: some View {
        HStack {
            Image(systemName: "lock").foregroundColor(.gray)
            if isPasswordVisible {
                TextField("Password", text: $password)
            } else {
                SecureField("Password", text: $password)
            }
            Button { isPasswordVisible.toggle() } label: {
                Image(systemName: isPasswordVisible ? "eye.slash" : "eye")
                    .foregroundColor(.gray)
            }
        }
        .animatedField(offset: -200, delay: 0.4, animate: animate)
        .fieldStyle(borderRed: passwordSubmitted && !password.isEmpty && !isPasswordValid)
    }
    
    private var submitButton: some View {
        Button(action: signUp) {
            Text("註冊")
                .font(.headline)
                .foregroundColor(.white)
                .frame(maxWidth: .infinity)
                .padding()
        }
        .background(
            LinearGradient(
                colors: isFormValid
                ? [Color(red: 65/255, green: 160/255, blue: 255/255),
                   Color(red: 120/255, green: 190/255, blue: 255/255)]
                : [Color.gray.opacity(0.6), Color.gray.opacity(0.3)],
                startPoint: .leading,
                endPoint: .trailing
            )
        )
        .clipShape(RoundedRectangle(cornerRadius: 10))
        .animatedField(offset: 50, delay: 0.4, animate: animate, inverse: true)
        .disabled(!isFormValid)
    }
    
    private var backButton: some ToolbarContent {
        ToolbarItem(placement: .navigationBarLeading) {
            Image(systemName: "chevron.left")
                .font(.title2.weight(.semibold))
                .onTapGesture { dismiss() }
        }
    }
    
    // MARK: - 動作
    private func signUp() {
        endEditingAndValidate()
        guard isFormValid else { return }
        
        isLoading = true
        Network.shared.signUp(email: email, userName: username, password: password) { result in
            DispatchQueue.main.async {
                isLoading = false
                switch result {
                case .success(let res):
                    responseMessage = res.error_code.localizedDescription
                    if res.error_code == .success { shouldDismiss = true }
                    showResponseAlert = true
                case .failure(let err):
                    errorMessage = err.localizedDescription
                }
            }
        }
    }
    
    private func endEditingAndValidate() {
        UIApplication.shared.sendAction(#selector(UIResponder.resignFirstResponder),
                                        to: nil, from: nil, for: nil)
        if !email.isEmpty    { emailSubmitted    = true }
        if !username.isEmpty { usernameSubmitted = true }
        if !password.isEmpty { passwordSubmitted = true }
    }
}

// MARK: - View 修飾器抽象
fileprivate extension View {
    /// 套用輸入框樣式
    func fieldStyle(borderRed: Bool) -> some View {
        self.padding()
            .background(Color.white.opacity(0.9))
            .cornerRadius(10)
            .shadow(color: .black.opacity(0.1), radius: 5, x: 0, y: 2)
            .overlay(
                RoundedRectangle(cornerRadius: 10)
                    .stroke(borderRed ? Color.red : Color.clear, lineWidth: 2)
            )
    }
    
    /// 以 offset + opacity + animation 的組合製作進場動畫
    func animatedField(offset: CGFloat,
                       delay: Double,
                       animate: Bool,
                       inverse: Bool = false) -> some View {
        let realOffset = inverse ? offset : -offset
        return self
            .offset(x: animate ? 0 : realOffset, y: animate ? 0 : max(0, realOffset))
            .opacity(animate ? 1 : 0)
            .animation(.easeOut(duration: 0.5).delay(delay), value: animate)
    }
}
#Preview {
    SignUpView()
}
