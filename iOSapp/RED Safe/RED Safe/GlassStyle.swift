import SwiftUI

/// 全域 App 背景：與儀表板一致的漸層與光暈效果。
struct AppBackground: View {
    @Binding private var animate: Bool
    @Environment(\.colorScheme) private var colorScheme

    init(animate: Binding<Bool> = .constant(true)) {
        _animate = animate
    }

    var body: some View {
        let largeGlowOpacity = colorScheme == .dark ? 0.22 : 0.45
        let smallGlowOpacity = colorScheme == .dark ? 0.14 : 0.32

        return ZStack {
            LinearGradient(
                colors: [.appBackgroundTop, .appBackgroundMid, .appBackgroundBottom],
                startPoint: .topLeading,
                endPoint: .bottomTrailing
            )

            Circle()
                .fill(Color.white.opacity(largeGlowOpacity))
                .frame(width: animate ? 360 : 220)
                .blur(radius: 60)
                .offset(x: -150, y: animate ? -260 : -140)
                .animation(.easeOut(duration: 1.0), value: animate)

            Circle()
                .fill(Color.white.opacity(smallGlowOpacity))
                .frame(width: animate ? 320 : 200)
                .blur(radius: 52)
                .offset(x: 170, y: animate ? 280 : 160)
                .animation(.easeOut(duration: 1.0).delay(0.05), value: animate)
        }
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
