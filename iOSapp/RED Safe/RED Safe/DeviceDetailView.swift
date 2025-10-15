import SwiftUI

/// Edge 詳細頁面：提供重新命名、更新密碼、解除綁定操作。
struct DeviceDetailView: View {
    let edge: EdgeSummary
    let rename: () -> Void
    let updatePassword: () -> Void
    let unbind: () -> Void

    var body: some View {
        List {
            Section("裝置資訊") {
                VStack(alignment: .leading, spacing: 6) {
                    Text(edge.displayName.isEmpty ? "未命名裝置" : edge.displayName)
                        .font(.title3.weight(.semibold))
                    Text(edge.edgeId)
                        .font(.footnote.monospaced())
                        .foregroundColor(.secondary)
                }
                .padding(.vertical, 6)
            }

            Section("狀態") {
                let online = edge.isOnline ?? false
                let color = online ? Color.green : Color.red
                let icon = online ? "wifi" : "wifi.slash"
                let text = online ? "在線" : "離線"

                HStack(spacing: 10) {
                    Image(systemName: icon)
                        .foregroundColor(color)
                    Text(text)
                        .font(.body.weight(.medium))
                        .foregroundColor(color)
                }
                .padding(.vertical, 4)
            }

            Section("管理") {
                ActionRow(title: "重新命名", icon: "square.and.pencil", action: rename)
                ActionRow(title: "更新 Edge 密碼", icon: "key.fill", action: updatePassword)
                ActionRow(title: "解除綁定", icon: "link.badge.minus", tint: .red, action: unbind)
            }

            Section("網路配置") {
                NavigationLink {
                    NetworkConfigView(edge: edge)
                } label: {
                    HStack {
                        Image(systemName: "network")
                        Text("網路配置")
                        Spacer()
                    }
                    .font(.body.weight(.semibold))
                    .padding(.vertical, 8)
                }
            }

            Section("IP Camera配置") {
                NavigationLink {
                    IPCameraConfigView(edge: edge)
                } label: {
                    HStack {
                        Image(systemName: "camera.on.rectangle")
                        Text("IP Camera配置")
                        Spacer()
                    }
                    .font(.body.weight(.semibold))
                    .padding(.vertical, 8)
                }
            }
        }
        .listStyle(.insetGrouped)
        .scrollContentBackground(.hidden)
        .background(
            LinearGradient(
                colors: [
                    Color(red: 247/255, green: 250/255, blue: 255/255),
                    Color(red: 228/255, green: 236/255, blue: 250/255)
                ],
                startPoint: .topLeading,
                endPoint: .bottomTrailing
            )
        )
        .listRowBackground(Color.white.opacity(0.9))
        .navigationTitle(edge.displayName.isEmpty ? edge.edgeId : edge.displayName)
        .navigationBarTitleDisplayMode(.inline)
    }
}

/// 單列操作按鈕。
private struct ActionRow: View {
    let title: String
    let icon: String
    var tint: Color = .accentColor
    let action: () -> Void

    var body: some View {
        Button(action: action) {
            HStack {
                Image(systemName: icon)
                Text(title)
                Spacer()
                Image(systemName: "chevron.right")
                    .foregroundStyle(.tertiary)
            }
            .font(.body.weight(.semibold))
            .foregroundColor(tint == .accentColor ? .primary : tint)
            .padding(.vertical, 8)
        }
        .buttonStyle(.plain)
    }
}

#Preview {
    let edge = EdgeSummary(edgeId: "RED-AAAA0000", displayName: "測試 Edge", isOnline: true)
    return NavigationStack {
        DeviceDetailView(
            edge: edge,
            rename: {},
            updatePassword: {},
            unbind: {}
        )
    }
}
