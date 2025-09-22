import SwiftUI
import UIKit

// MARK: - 裝置分頁

/// Sheet 狀態列舉：新增、改名、更新密碼三種情境。
enum DeviceSheet: Identifiable {
    case bind
    case rename(EdgeSummary)
    case password(EdgeSummary)

    var id: String {
        switch self {
        case .bind: return "bind"
        case .rename(let edge): return "rename-\(edge.edgeId)"
        case .password(let edge): return "password-\(edge.edgeId)"
        }
    }
}

/// DashboardView 管理 Edge 裝置相關 UI 與互動。
struct DashboardView: View {
    @ObservedObject var homeVM: HomeViewModel
    @ObservedObject var auth: AuthManager

    @Binding var deviceSheet: DeviceSheet?
    @Binding var deviceToUnbind: EdgeSummary?
    @Binding var showUnbindConfirm: Bool
    @Binding var animateBackground: Bool

    @State private var navigationPath = NavigationPath()

    var body: some View {
        NavigationStack(path: $navigationPath) {
            GeometryReader { proxy in
                ZStack {
                    background

                    VStack(spacing: 20) {
                        header
                            .padding(.top, proxy.safeAreaInsets.top + 6)

                        summaryCard

                        deviceContent
                            .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .top)
                    }
                    .padding(.horizontal, 20)
                    .padding(.bottom, 24 + proxy.safeAreaInsets.bottom)
                    .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .top)
                }
                .ignoresSafeArea()
                .onAppear { animateBackground = true }
            }
            .navigationDestination(for: EdgeSummary.self) { edge in
                DeviceDetailView(
                    edge: edge,
                    rename: { deviceSheet = .rename(edge) },
                    updatePassword: { deviceSheet = .password(edge) },
                    unbind: {
                        deviceToUnbind = edge
                        showUnbindConfirm = true
                    }
                )
            }
        }
        .sheet(item: $deviceSheet) { sheet in
            switch sheet {
            case .bind:
                BindEdgeSheet { edgeId, name, password in
                    homeVM.bindEdge(edgeId: edgeId, name: name, password: password)
                }
            case .rename(let edge):
                RenameEdgeSheet(edge: edge) { newName in
                    homeVM.renameEdge(edgeId: edge.edgeId, newName: newName)
                }
            case .password(let edge):
                EdgePasswordSheet(edge: edge) { current, newPassword in
                    homeVM.updateEdgePassword(edgeId: edge.edgeId, currentPassword: current, newPassword: newPassword)
                }
            }
        }
        .confirmationDialog(
            "確定要解除綁定這台 Edge 嗎？",
            isPresented: $showUnbindConfirm,
            titleVisibility: .visible
        ) {
            Button("解除綁定", role: .destructive) {
                guard let deviceToUnbind else { return }
                homeVM.unbindEdge(edgeId: deviceToUnbind.edgeId)
                self.deviceToUnbind = nil
            }
            Button("取消", role: .cancel) {
                deviceToUnbind = nil
            }
        } message: {
            Text("解除綁定後，需要重新輸入 Edge 密碼才能再次連結。")
        }
    }

    // MARK: - 子區塊

    private var background: some View {
        ZStack {
            LinearGradient(
                colors: [
                    Color(red: 246/255, green: 250/255, blue: 255/255),
                    Color(red: 223/255, green: 235/255, blue: 250/255),
                    Color(red: 205/255, green: 222/255, blue: 245/255)
                ],
                startPoint: .topLeading,
                endPoint: .bottomTrailing
            )
            .ignoresSafeArea()

            Circle()
                .fill(Color.white.opacity(0.6))

            Color(.systemGroupedBackground)
                .ignoresSafeArea()

            Circle()
                .fill(Color.white.opacity(0.45))
                .frame(width: animateBackground ? 360 : 220)
                .blur(radius: 60)
                .offset(x: -150, y: animateBackground ? -260 : -140)
                .animation(.easeOut(duration: 1.0), value: animateBackground)

            Circle()
                .fill(Color.white.opacity(0.32))
                .frame(width: animateBackground ? 320 : 200)
                .blur(radius: 52)
                .offset(x: 170, y: animateBackground ? 280 : 160)
                .animation(.easeOut(duration: 1.0).delay(0.05), value: animateBackground)
        }
    }

    private var header: some View {
        HStack(spacing: 16) {
            VStack(alignment: .leading, spacing: 8) {
                Text("你好，\(auth.userName ?? "使用者")")
                    .font(.system(size: 30, weight: .bold, design: .rounded))
                    .foregroundColor(.primary)
                Text("管理並監控已綁定的 Edge 裝置")
                    .font(.subheadline)
                    .foregroundColor(.secondary)
            }
            Spacer()
            quickActionButton(icon: "arrow.clockwise") {
                homeVM.loadEdges(showIndicator: true)
            }
        }
    }

    private var summaryCard: some View {
        let count = homeVM.edges.count

        return HStack(spacing: 16) {
            VStack(alignment: .leading, spacing: 10) {
                Text("已綁定 Edge")
                    .font(.caption.weight(.semibold))
                    .foregroundStyle(Color.secondary)
                HStack(alignment: .firstTextBaseline, spacing: 8) {
                    Text("\(count)")
                        .font(.system(size: 44, weight: .bold, design: .rounded))
                    Text("台")
                        .font(.title3)
                        .foregroundStyle(Color.secondary)
                }
                Text(count == 0 ? "尚未綁定任何 Edge 裝置" : "點選下方裝置卡片以快速管理 Edge")
                    .font(.footnote)
                    .foregroundStyle(Color.secondary)
            }
            .frame(maxWidth: .infinity, alignment: .leading)

            Button {
                deviceSheet = .bind
            } label: {
                Image(systemName: "plus")
                    .font(.headline)
                    .padding(.horizontal, 18)
                    .padding(.vertical, 10)
                    .background(
                        Capsule().fill(
                            LinearGradient(
                                colors: [
                                    Color(red: 118/255, green: 186/255, blue: 255/255),
                                    Color(red: 78/255, green: 156/255, blue: 255/255)
                                ],
                                startPoint: .topLeading,
                                endPoint: .bottomTrailing
                            )
                        )
                    )
                    .overlay(
                        Capsule()
                            .stroke(Color.white.opacity(0.55))
                    )
                    .foregroundStyle(Color.white)
            }
            .buttonStyle(.plain)
        }
        .padding(24)
        .glassCard(cornerRadius: 30)
        .padding(.horizontal, 4)
    }

    private var deviceContent: some View {
        Group {
            if homeVM.isLoading && homeVM.edges.isEmpty {
                ProgressView()
                    .progressViewStyle(.circular)
                    .tint(.white)
                    .padding(.top, 40)
            } else if homeVM.edges.isEmpty {
                emptyState
            } else {
                ScrollView(showsIndicators: false) {
                    LazyVStack(spacing: 18) {
                        ForEach(homeVM.edges) { edge in
                            NavigationLink(value: edge) {
                                EdgeCard(edge: edge)
                            }
                            .buttonStyle(.plain)
                        }
                    }
                    .padding(.bottom, 32)
                }
                .refreshable {
                    homeVM.loadEdges(showIndicator: false)
                }
            }
        }
    }

    private var emptyState: some View {
        VStack(spacing: 18) {
            Image(systemName: "shippingbox.fill")
                .font(.system(size: 60))
                .foregroundStyle(Color.accentColor)
            Text("還沒有綁定任何 Edge 裝置")
                .font(.headline)
                .foregroundStyle(Color.primary)
            Text("點擊下方按鈕開始綁定 Edge 裝置")
                .font(.footnote)
                .foregroundStyle(Color.secondary)
                .multilineTextAlignment(.center)
                .padding(.horizontal, 12)
            Button {
                deviceSheet = .bind
            } label: {
                Text("立即綁定")
                    .font(.headline)
                    .foregroundStyle(Color.white)
                    .padding(.horizontal, 32)
                    .padding(.vertical, 12)
                    .background(
                        Capsule().fill(
                            LinearGradient(
                                colors: [
                                    Color(red: 118/255, green: 186/255, blue: 255/255),
                                    Color(red: 78/255, green: 156/255, blue: 255/255)
                                ],
                                startPoint: .topLeading,
                                endPoint: .bottomTrailing
                            )
                        )
                    )
                    .overlay(Capsule().stroke(Color.white.opacity(0.6)))
            }
            .buttonStyle(.plain)
        }
        .padding(36)
        .frame(maxWidth: .infinity)
        .background(
            RoundedRectangle(cornerRadius: 32, style: .continuous)
                .fill(Color.white.opacity(0.92))
        )
        .overlay(
            RoundedRectangle(cornerRadius: 32, style: .continuous)
                .stroke(Color.white.opacity(0.4))
        )
        .shadow(color: Color.black.opacity(0.08), radius: 14, x: 0, y: 10)
    }

    private func quickActionButton(icon: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Image(systemName: icon)
                .font(.headline)
                .foregroundStyle(Color.accentColor)
                .frame(width: 44, height: 44)
                .background(Circle().fill(Color.white.opacity(0.9)))
        }
        .buttonStyle(.plain)
        .shadow(color: Color.black.opacity(0.1), radius: 6, x: 0, y: 4)
    }
}

// MARK: - 元件

private struct EdgeCard: View {
    let edge: EdgeSummary

    var body: some View {
        let online = edge.isOnline ?? false
        let statusColor = online ? Color.green.opacity(0.85) : Color.red.opacity(0.8)
        let statusText = online ? "在線" : "離線"
        let statusIcon = online ? "wifi" : "wifi.slash"

        return VStack(alignment: .leading, spacing: 18) {
            HStack(alignment: .top, spacing: 16) {
                LinearGradient(
                    colors: [
                        Color(red: 0/255, green: 187/255, blue: 255/255),
                        Color(red: 0/255, green: 120/255, blue: 255/255)
                    ],
                    startPoint: .topLeading,
                    endPoint: .bottomTrailing
                )
                .frame(width: 58, height: 58)
                .clipShape(RoundedRectangle(cornerRadius: 18, style: .continuous))
                .shadow(color: Color.black.opacity(0.18), radius: 10, x: 0, y: 8)
                .overlay(
                    Image(systemName: "dot.radiowaves.left.and.right")
                        .foregroundStyle(.white)
                        .font(.title2)
                )
                VStack(alignment: .leading, spacing: 10) {
                    Text(edge.displayName.isEmpty ? "未命名裝置" : edge.displayName)
                        .font(.system(size: 22, weight: .semibold, design: .rounded))
                        .foregroundStyle(Color.primary)
                    Text(edge.edgeId)
                        .font(.footnote.monospaced())
                        .foregroundStyle(Color.secondary)
                }
                Spacer()
            }
            Divider()
                .background(Color.gray.opacity(0.15))
            HStack(spacing: 8) {
                Image(systemName: statusIcon)
                    .foregroundStyle(statusColor)
                Text(statusText)
                    .font(.footnote.weight(.medium))
                    .foregroundStyle(statusColor)
            }
            .padding(.top, 4)
        }
        .padding(22)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(
            RoundedRectangle(cornerRadius: 30, style: .continuous)
                .fill(
                    LinearGradient(
                        colors: [Color.white.opacity(0.90), Color.white.opacity(0.88)],
                        startPoint: .topLeading,
                        endPoint: .bottomTrailing
                    )
                )
        )
        .overlay(
            RoundedRectangle(cornerRadius: 30, style: .continuous)
                .stroke(Color.white.opacity(0.28), lineWidth: 1)
        )
        .padding(.horizontal, 4)
    }
}

// MARK: - 互動表單
private struct BindEdgeSheet: View {
    @Environment(\.dismiss) private var dismiss
    @State private var edgeId = ""
    @State private var displayName = ""
    @State private var edgePassword = ""

    var onSubmit: (String, String, String) -> Void

    private var normalizedEdgeId: String { edgeId.trimmed.uppercased() }
    private var isEdgeIdValid: Bool {
        let pattern = "^RED-[A-F0-9]{8}$"
        return NSPredicate(format: "SELF MATCHES %@", pattern).evaluate(with: normalizedEdgeId)
    }
    private var isNameValid: Bool { !displayName.trimmed.isEmpty && displayName.trimmed.count <= 16 }
    private var isFormValid: Bool { isEdgeIdValid && isNameValid && !edgePassword.trimmed.isEmpty }

    var body: some View {
        NavigationStack {
            Form {
                Section("Edge 資訊") {
                    TextField("Edge ID (RED-XXXXXXXX)", text: $edgeId)
                        .textInputAutocapitalization(.characters)
                        .autocorrectionDisabled()
                        .onChangeCompat(of: edgeId) { _, newValue in
                            if newValue != newValue.uppercased() {
                                edgeId = newValue.uppercased()
                            }
                        }
                    TextField("顯示名稱", text: $displayName)
                        .onChangeCompat(of: displayName) { _, value in
                            if value.count > 16 {
                                displayName = String(value.prefix(16))
                            }
                        }
                    SecureField("Edge 密碼", text: $edgePassword)
                }
                if !isEdgeIdValid {
                    Text("Edge ID 必須為 RED- 開頭的 8 碼十六進位字元")
                        .font(.footnote)
                        .foregroundColor(.red)
                }
                if !isNameValid {
                    Text("裝置名稱需為 1-16 字元")
                        .font(.footnote)
                        .foregroundColor(.red)
                }
            }
            .navigationTitle("綁定 Edge")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("取消") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("綁定") {
                        onSubmit(normalizedEdgeId, displayName.trimmed, edgePassword.trimmed)
                        dismiss()
                    }
                    .disabled(!isFormValid)
                }
            }
        }
    }
}

private struct RenameEdgeSheet: View {
    @Environment(\.dismiss) private var dismiss
    @State private var newName: String

    let edge: EdgeSummary
    let onSubmit: (String) -> Void

    init(edge: EdgeSummary, onSubmit: @escaping (String) -> Void) {
        self.edge = edge
        self.onSubmit = onSubmit
        _newName = State(initialValue: edge.displayName)
    }

    private var trimmedName: String { newName.trimmed }
    private var isValid: Bool { !trimmedName.isEmpty && trimmedName.count <= 16 }

    var body: some View {
        NavigationStack {
            Form {
                Section("重新命名 Edge") {
                    TextField("顯示名稱", text: $newName)
                        .onChangeCompat(of: newName) { _, value in
                            if value.count > 16 {
                                newName = String(value.prefix(16))
                            }
                        }
                }
                if !isValid {
                    Text("名稱需為 1-16 字元")
                        .font(.footnote)
                        .foregroundColor(.red)
                }
            }
            .navigationTitle("重新命名")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("取消") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("儲存") {
                        onSubmit(trimmedName)
                        dismiss()
                    }
                    .disabled(!isValid)
                }
            }
        }
    }
}

private struct EdgePasswordSheet: View {
    @Environment(\.dismiss) private var dismiss
    @State private var currentPassword = ""
    @State private var newPassword = ""
    @State private var confirmPassword = ""

    let edge: EdgeSummary
    let onSubmit: (String, String) -> Void

    private var trimmedCurrent: String { currentPassword.trimmed }
    private var trimmedNew: String { newPassword.trimmed }
    private var trimmedConfirm: String { confirmPassword.trimmed }
    private var isValid: Bool { !trimmedCurrent.isEmpty && !trimmedNew.isEmpty && trimmedNew == trimmedConfirm }

    var body: some View {
        NavigationStack {
            Form {
                Section("更新 Edge 密碼") {
                    SecureField("目前密碼", text: $currentPassword)
                    SecureField("新密碼", text: $newPassword)
                    SecureField("確認新密碼", text: $confirmPassword)
                }
                if trimmedNew != trimmedConfirm {
                    Text("兩次輸入的新密碼不同")
                        .font(.footnote)
                        .foregroundColor(.red)
                }
            }
            .navigationTitle(edge.displayName.isEmpty ? edge.edgeId : edge.displayName)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("取消") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("更新") {
                        onSubmit(trimmedCurrent, trimmedNew)
                        dismiss()
                    }
                    .disabled(!isValid)
                }
            }
        }
    }
}

// MARK: - Backward-compatible onChange helper

private extension View {
    @ViewBuilder
    func onChangeCompat<Value: Equatable>(
        of value: Value,
        perform action: @escaping (_ oldValue: Value, _ newValue: Value) -> Void
    ) -> some View {
        if #available(iOS 17.0, *) {
            self.onChange(of: value) { oldValue, newValue in
                action(oldValue, newValue)
            }
        } else {
            // Fallback for iOS 16 及更早版本
            self.onChange(of: value) { newValue in
                action(newValue, newValue)
            }
        }
    }
}
