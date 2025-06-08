import SwiftUI

struct LoadView: View {
    var body: some View {
        ZStack {
            LinearGradient(
                gradient: Gradient(colors: [
                    Color(red: 240/255, green: 248/255, blue: 255/255),
                    Color(red: 210/255, green: 235/255, blue: 250/255)
                ]),
                startPoint: .topLeading,
                endPoint: .bottomTrailing
            )
            .ignoresSafeArea()

            VStack(spacing: 20) {
                Image("RED_Safe_icon1")
                    .resizable()
                    .scaledToFit()
                    .frame(width: 200, height: 200)

                ProgressView()  // 加上系統圓形指示器
                    .scaleEffect(1.5)
            }
        }
    }
}

#Preview {
    LoadView()
}
