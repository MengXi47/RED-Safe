import SwiftUI

/// 共用的 Liquid Glass 介面風格工具。
struct LiquidGlassBackground: View {
    var body: some View {
        Color(.systemGroupedBackground)
            .overlay(
                ZStack {
                    Circle()
                        .fill(Color.white.opacity(0.45))
                        .frame(width: 360)
                        .blur(radius: 60)
                        .offset(x: -150, y: -220)
                    Circle()
                        .fill(Color.white.opacity(0.32))
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

    func body(content: Content) -> some View {
        content
            .background(
                RoundedRectangle(cornerRadius: cornerRadius, style: .continuous)
                    .fill(Color.white.opacity(opacity))
                    .overlay(
                        RoundedRectangle(cornerRadius: cornerRadius, style: .continuous)
                            .stroke(Color.white.opacity(0.5), lineWidth: 1)
                    )
            )
            .clipShape(RoundedRectangle(cornerRadius: cornerRadius, style: .continuous))
            .shadow(color: Color.black.opacity(0.06), radius: 12, x: 0, y: 6)
    }
}

extension View {
    /// 套用 Liquid Glass 卡片風格。
    func glassCard(cornerRadius: CGFloat = 28, opacity: Double = 0.92) -> some View {
        modifier(GlassCardModifier(cornerRadius: cornerRadius, opacity: opacity))
    }
}
