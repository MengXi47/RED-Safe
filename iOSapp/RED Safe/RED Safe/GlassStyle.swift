import SwiftUI

/// 共用的 Liquid Glass 介面風格工具。
struct LiquidGlassBackground: View {
    @Environment(\.colorScheme) private var colorScheme

    var body: some View {
        LinearGradient(
            colors: [.appBackgroundTop, .appBackgroundMid, .appBackgroundBottom],
            startPoint: .topLeading,
            endPoint: .bottomTrailing
        )
        .overlay(
            ZStack {
                Circle()
                    .fill(colorScheme == .dark ? Color.white.opacity(0.18) : Color.white.opacity(0.45))
                    .frame(width: 360)
                    .blur(radius: 60)
                    .offset(x: -150, y: -220)
                Circle()
                    .fill(colorScheme == .dark ? Color.white.opacity(0.12) : Color.white.opacity(0.32))
                    .frame(width: 300)
                    .blur(radius: 52)
                    .offset(x: 180, y: 260)
            }
        )
        .ignoresSafeArea()
    }
}

private struct GlassCardModifier: ViewModifier {
    var cornerRadius: CGFloat
    var opacity: Double
    @Environment(\.colorScheme) private var colorScheme

    func body(content: Content) -> some View {
        let opacityMultiplier = opacity / 0.92
        let fillColor = colorScheme == .dark ? Color.surfaceBackground : Color.surfaceBackground.opacity(opacityMultiplier)

        content
            .background(
                RoundedRectangle(cornerRadius: cornerRadius, style: .continuous)
                    .fill(fillColor)
                    .overlay(
                        RoundedRectangle(cornerRadius: cornerRadius, style: .continuous)
                            .stroke(Color.surfaceStroke, lineWidth: 1)
                    )
            )
            .clipShape(RoundedRectangle(cornerRadius: cornerRadius, style: .continuous))
            .shadow(color: Color.surfaceShadow, radius: 12, x: 0, y: 6)
    }
}

extension View {
    /// 套用 Liquid Glass 卡片風格。
    func glassCard(cornerRadius: CGFloat = 28, opacity: Double = 0.92) -> some View {
        modifier(GlassCardModifier(cornerRadius: cornerRadius, opacity: opacity))
    }
}
