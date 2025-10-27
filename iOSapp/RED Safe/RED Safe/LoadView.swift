import SwiftUI

struct LoadView: View {
    @State private var animate = false

    var body: some View {
        ZStack {
            LinearGradient(
                colors: [.appBackgroundTop, .appBackgroundMid, .appBackgroundBottom],
                startPoint: animate ? .topTrailing : .topLeading,
                endPoint: animate ? .bottomLeading : .bottomTrailing
            )
            .ignoresSafeArea()
            .animation(.easeInOut(duration: 2.4).repeatForever(autoreverses: true), value: animate)

            VStack(spacing: 24) {
                ZStack {
                    RoundedRectangle(cornerRadius: 40, style: .continuous)
                        .fill(Color.loadingBackdrop)
                        .frame(width: 220, height: 220)
                        .blur(radius: 12)
                    Image("RED_Safe_icon1")
                        .resizable()
                        .scaledToFit()
                        .frame(width: 160, height: 160)
                        .shadow(color: Color.loadingShadow, radius: 22, x: 0, y: 18)
                        .scaleEffect(animate ? 1.02 : 0.98)
                        .animation(.easeInOut(duration: 1.2).repeatForever(autoreverses: true), value: animate)
                }

                VStack(spacing: 8) {
                    Text("RED Safe")
                        .font(.title2.weight(.bold))
                        .foregroundColor(.primary)
                    Text("正在為您建立安全連線")
                        .font(.callout)
                        .foregroundColor(.secondary)
                }

                ProgressView()
                    .progressViewStyle(.circular)
                    .tint(.white)
                    .scaleEffect(1.4)
            }
            .offset(y: -40)
        }
        .onAppear { animate = true }
    }
}

#Preview {
    LoadView()
}
