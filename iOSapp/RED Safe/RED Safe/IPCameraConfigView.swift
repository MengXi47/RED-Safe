import SwiftUI

struct IPCameraConfigView: View {
    let edge: EdgeSummary

    @State private var devices: [IPCameraDeviceDTO] = []
    @State private var isLoading = false
    @State private var errorMessage: String?
    @State private var lastTraceId: String?

    var body: some View {
        List {
            Section {
                VStack(alignment: .leading, spacing: 8) {
                    Text("在 Edge 上執行 IP Camera 掃描，取得當前可用的攝影機資訊。")
                        .font(.footnote)
                        .foregroundColor(.secondary)
                    Button {
                        Task { await triggerScan() }
                    } label: {
                        HStack {
                            if isLoading {
                                ProgressView()
                                    .progressViewStyle(.circular)
                            }
                            Text(isLoading ? "掃描中…" : "開始掃描")
                                .fontWeight(.semibold)
                        }
                        .frame(maxWidth: .infinity)
                    }
                    .buttonStyle(.borderedProminent)
                    .disabled(isLoading)
                }
                .padding(.vertical, 4)
            }

            if let message = errorMessage {
                Section {
                    Label(message, systemImage: "exclamationmark.triangle.fill")
                        .foregroundColor(.orange)
                        .multilineTextAlignment(.leading)
                }
            }

            Section("掃描結果") {
                if devices.isEmpty {
                    Text(isLoading ? "正在取得資料…" : "尚未取得資料，請點擊「開始掃描」。")
                        .foregroundColor(.secondary)
                } else {
                    ForEach(devices) { device in
                        VStack(alignment: .leading, spacing: 6) {
                            HStack {
                                Label(device.name, systemImage: "camera.fill")
                                    .font(.headline)
                            }
                            VStack(alignment: .leading, spacing: 4) {
                                InfoRow(title: "IP", value: device.ip)
                                InfoRow(title: "MAC", value: device.mac)
                            }
                        }
                        .padding(.vertical, 6)
                    }
                }
            }
        }
        .navigationTitle("IP Camera配置")
        .navigationBarTitleDisplayMode(.inline)
    }

    private func triggerScan() async {
        await MainActor.run {
            isLoading = true
            errorMessage = nil
        }
        do {
            let command = try await APIClient.shared.sendEdgeCommand(edgeId: edge.edgeId, code: "101")
            let result = try await APIClient.shared.fetchEdgeCommandResult(traceId: command.traceId)
            await MainActor.run {
                self.devices = result.result
                self.lastTraceId = result.traceId ?? command.traceId
                self.isLoading = false
                self.errorMessage = nil
            }
        } catch {
            await MainActor.run {
                self.devices = []
                self.isLoading = false
                self.errorMessage = error.localizedDescription
            }
        }
    }
}

private struct InfoRow: View {
    let title: String
    let value: String

    var body: some View {
        HStack {
            Text(title)
                .font(.subheadline.weight(.medium))
                .foregroundColor(.secondary)
                .frame(width: 44, alignment: .leading)
            Text(value)
                .font(.subheadline.monospaced())
                .foregroundColor(.primary)
        }
    }
}

#Preview {
    NavigationStack {
        IPCameraConfigView(edge: EdgeSummary(edgeId: "RED-TEST0001", displayName: "測試 Edge", isOnline: true))
    }
}
