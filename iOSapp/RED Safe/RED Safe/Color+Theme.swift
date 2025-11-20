import SwiftUI
import UIKit

/// 集中管理會隨著深淺色模式切換的顏色，避免在各畫面重複硬編碼數值。
extension Color {
    private static func adaptive(light: UIColor, dark: UIColor) -> Color {
        Color(
            UIColor { traitCollection in
                traitCollection.userInterfaceStyle == .dark ? dark : light
            }
        )
    }

    // 全域背景（儀表板漸層）
    static let appBackgroundTop = adaptive(
        light: UIColor(red: 246/255, green: 250/255, blue: 255/255, alpha: 1),
        dark: UIColor(red: 16/255, green: 20/255, blue: 33/255, alpha: 1)
    )
    static let appBackgroundMid = adaptive(
        light: UIColor(red: 223/255, green: 235/255, blue: 250/255, alpha: 1),
        dark: UIColor(red: 13/255, green: 18/255, blue: 32/255, alpha: 1)
    )
    static let appBackgroundBottom = adaptive(
        light: UIColor(red: 205/255, green: 222/255, blue: 245/255, alpha: 1),
        dark: UIColor(red: 9/255, green: 14/255, blue: 26/255, alpha: 1)
    )

    // 卡片 / 區塊背景
    static let surfaceBackground = adaptive(
        light: UIColor(white: 1.0, alpha: 0.92),
        dark: UIColor(red: 32/255, green: 37/255, blue: 52/255, alpha: 0.92)
    )
    static let surfaceStroke = adaptive(
        light: UIColor.white.withAlphaComponent(0.35),
        dark: UIColor.white.withAlphaComponent(0.12)
    )
    static let surfaceShadow = adaptive(
        light: UIColor.black.withAlphaComponent(0.06),
        dark: UIColor.black.withAlphaComponent(0.55)
    )

    static let deviceCardTop = adaptive(
        light: UIColor(white: 1.0, alpha: 0.95),
        dark: UIColor(red: 43/255, green: 51/255, blue: 69/255, alpha: 1)
    )
    static let deviceCardBottom = adaptive(
        light: UIColor(white: 0.96, alpha: 0.92),
        dark: UIColor(red: 30/255, green: 36/255, blue: 50/255, alpha: 1)
    )

    static let emptyStateBackground = adaptive(
        light: UIColor(white: 1.0, alpha: 0.92),
        dark: UIColor(red: 25/255, green: 30/255, blue: 45/255, alpha: 0.92)
    )

    static let controlBackground = adaptive(
        light: UIColor.white.withAlphaComponent(0.9),
        dark: UIColor.white.withAlphaComponent(0.08)
    )

    static let separatorMuted = adaptive(
        light: UIColor.black.withAlphaComponent(0.06),
        dark: UIColor.white.withAlphaComponent(0.1)
    )

    static let fieldBackground = adaptive(
        light: UIColor(white: 1.0, alpha: 0.95),
        dark: UIColor(red: 35/255, green: 40/255, blue: 56/255, alpha: 0.95)
    )

    static let outlineMuted = adaptive(
        light: UIColor.black.withAlphaComponent(0.08),
        dark: UIColor.white.withAlphaComponent(0.18)
    )

    static let actionStroke = adaptive(
        light: UIColor.white.withAlphaComponent(0.4),
        dark: UIColor(red: 130/255, green: 190/255, blue: 255/255, alpha: 0.65)
    )

    static let heroShadow = adaptive(
        light: UIColor.black.withAlphaComponent(0.24),
        dark: UIColor.black.withAlphaComponent(0.65)
    )

    static let iconCircleBackground = adaptive(
        light: UIColor.white.withAlphaComponent(0.22),
        dark: UIColor.white.withAlphaComponent(0.14)
    )

    static let detailTextPrimary = adaptive(
        light: UIColor(red: 24/255, green: 36/255, blue: 62/255, alpha: 1),
        dark: UIColor(red: 94/255, green: 138/255, blue: 209/255, alpha: 1)
    )

    static let detailTextSecondary = adaptive(
        light: UIColor(red: 92/255, green: 108/255, blue: 138/255, alpha: 1),
        dark: UIColor(red: 132/255, green: 166/255, blue: 214/255, alpha: 1)
    )

    static let loadingBackdrop = adaptive(
        light: UIColor.white.withAlphaComponent(0.38),
        dark: UIColor.white.withAlphaComponent(0.12)
    )

    static let loadingShadow = adaptive(
        light: UIColor.black.withAlphaComponent(0.2),
        dark: UIColor.black.withAlphaComponent(0.55)
    )
}
