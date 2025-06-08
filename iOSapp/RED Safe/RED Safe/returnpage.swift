import SwiftUI
import UIKit

// 隱藏 .navigationBarBackButtonHidden(true) 後依然保留滑動返回功能
extension UINavigationController: @retroactive UIGestureRecognizerDelegate {
    open override func viewDidLoad() {
        super.viewDidLoad()
        // 讓系統的 interactivePopGestureRecognizer 在隱藏 Back 按鈕時仍可用
        interactivePopGestureRecognizer?.delegate = self
    }

    // 避免在 rootViewController 也啟動手勢
    public func gestureRecognizerShouldBegin(_ gestureRecognizer: UIGestureRecognizer) -> Bool {
        viewControllers.count > 1
    }
}

