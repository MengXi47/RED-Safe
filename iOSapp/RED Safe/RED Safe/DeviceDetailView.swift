import SwiftUI

/// Edge 詳細頁面：提供裝置資訊、配置連結與管理操作。
struct DeviceDetailView: View {
    let edge: EdgeSummary
    let rename: () -> Void
    let updatePassword: () -> Void
    let unbind: () -> Void

    @Environment(\.colorScheme) private var colorScheme

    var body: some View {
        ScrollView(showsIndicators: false) {
            VStack(spacing: 24) {
                infoCard
                configurationCard
                managementCard
            }
            .padding(.horizontal, 20)
            .padding(.vertical, 32)
        }
        .background { AppBackground() }
        .navigationTitle(edge.displayName.isEmpty ? edge.edgeId : edge.displayName)
        .navigationBarTitleDisplayMode(.inline)
        .toolbar(.hidden, for: .tabBar)
    }

    private let rowIconSize: CGFloat = 20
    private let rowIconWidth: CGFloat = 30

    private var infoCard: some View {
        cardContainer {
            VStack(alignment: .leading, spacing: 22) {
                VStack(alignment: .leading, spacing: 8) {
                    Text(edge.displayName.isEmpty ? "未命名裝置" : edge.displayName)
                        .font(.system(size: 27, weight: .bold, design: .rounded))
                        .foregroundStyle(primaryTextColor)
                    Text(edge.edgeId)
                        .font(.system(size: 14, weight: .medium, design: .monospaced))
                        .foregroundStyle(secondaryTextColor)
                }

                Divider().overlay(dividerColor)

                let online = edge.isOnline ?? false
                let statusColor = online ? Color.green.opacity(0.9) : Color.red.opacity(0.85)
                let statusIcon = online ? "wifi" : "wifi.slash"
                let statusText = online ? "在線" : "離線"

                HStack(spacing: 12) {
                    Image(systemName: statusIcon)
                        .font(.system(size: 20, weight: .semibold))
                        .foregroundStyle(statusColor)
                    Text(statusText)
                        .font(.system(size: 17, weight: .semibold))
                        .foregroundStyle(statusColor)
                    Spacer()
                }
            }
        }
    }

    private var configurationCard: some View {
        cardContainer {
            VStack(alignment: .leading, spacing: 20) {
                Text("裝置配置")
                    .font(.system(size: 13, weight: .semibold))
                    .foregroundStyle(secondaryTextColor)

                VStack(spacing: 0) {
                    NavigationLink {
                        NetworkConfigView(edge: edge)
                    } label: {
                        navigationRow(icon: "network", title: "網路配置")
                    }
                    .buttonStyle(.plain)

                    Divider().overlay(dividerColor)

                    NavigationLink {
                        IPCameraConfigView(edge: edge)
                    } label: {
                        navigationRow(icon: "camera.on.rectangle", title: "IP Camera配置")
                    }
                    .buttonStyle(.plain)
                }
            }
        }
    }

    private var managementCard: some View {
        cardContainer {
            VStack(alignment: .leading, spacing: 20) {
                Text("裝置管理")
                    .font(.system(size: 13, weight: .semibold))
                    .foregroundStyle(secondaryTextColor)

                VStack(spacing: 0) {
                    ActionRow(title: "重新命名", icon: "square.and.pencil", action: rename)

                    Divider().overlay(dividerColor)

                    ActionRow(title: "更新 Edge 密碼", icon: "key.fill", action: updatePassword)

                    Divider().overlay(dividerColor)

                    ActionRow(title: "解除綁定", icon: "link.badge.minus", tint: .red, action: unbind)
                }
            }
        }
    }

    private var primaryTextColor: Color {
        colorScheme == .dark ? .white : Color.detailTextPrimary
    }

    private var secondaryTextColor: Color {
        colorScheme == .dark ? Color.white.opacity(0.75) : Color.detailTextSecondary
    }

    private var dividerColor: Color {
        colorScheme == .dark ? Color.white.opacity(0.18) : Color.white.opacity(0.28)
    }

    private func cardContainer<Content: View>(@ViewBuilder content: () -> Content) -> some View {
        content()
            .padding(28)
            .frame(maxWidth: .infinity, alignment: .leading)
            .background(
                RoundedRectangle(cornerRadius: 30, style: .continuous)
                    .fill(
                        LinearGradient(
                            colors: [Color.deviceCardTop, Color.deviceCardBottom],
                            startPoint: .topLeading,
                            endPoint: .bottomTrailing
                        )
                    )
            )
            .overlay(
                RoundedRectangle(cornerRadius: 30, style: .continuous)
                    .stroke(Color.surfaceStroke, lineWidth: 1)
            )
            .shadow(color: Color.surfaceShadow, radius: 14, x: 0, y: 10)
    }

    private func navigationRow(icon: String, title: String) -> some View {
        HStack(spacing: 14) {
            Image(systemName: icon)
                .font(.system(size: rowIconSize, weight: .semibold))
                .frame(width: rowIconWidth, alignment: .leading)
                .foregroundStyle(primaryTextColor)
            Text(title)
                .font(.system(size: 17, weight: .semibold))
                .foregroundStyle(primaryTextColor)
                .frame(maxWidth: .infinity, alignment: .leading)
            Spacer()
            Image(systemName: "chevron.right")
                .font(.system(size: 16, weight: .semibold))
                .foregroundStyle(secondaryTextColor)
        }
        .padding(.vertical, 12)
    }
}

/// 單列操作按鈕。
private struct ActionRow: View {
    let title: String
    let icon: String
    var tint: Color = .accentColor
    let action: () -> Void

    @Environment(\.colorScheme) private var colorScheme
    private let iconSize: CGFloat = 20
    private let iconWidth: CGFloat = 30

    private var baseColor: Color {
        if tint == .accentColor {
            return colorScheme == .dark ? .white : Color.detailTextPrimary
        }
        return tint
    }

    private var accessoryColor: Color {
        if tint == .accentColor {
            return colorScheme == .dark ? Color.white.opacity(0.75) : Color.detailTextSecondary
        }
        return tint
    }

    var body: some View {
        Button(action: action) {
            HStack(spacing: 14) {
                Image(systemName: icon)
                    .font(.system(size: iconSize, weight: .semibold))
                    .frame(width: iconWidth, alignment: .leading)
                    .foregroundStyle(baseColor)
                Text(title)
                    .font(.system(size: 17, weight: .semibold))
                    .foregroundStyle(baseColor)
                    .frame(maxWidth: .infinity, alignment: .leading)
                Spacer()
                Image(systemName: "chevron.right")
                    .font(.system(size: 16, weight: .semibold))
                    .foregroundStyle(accessoryColor)
            }
            .padding(.vertical, 12)
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
