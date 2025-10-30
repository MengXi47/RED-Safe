import SwiftUI

struct NetworkConfigView: View {
    let edge: EdgeSummary

    @State private var config: EdgeNetworkConfigDTO?
    @State private var isLoading = false
    @State private var errorMessage: String?

    var body: some View {
        List {
            Section {
                VStack(alignment: .leading, spacing: 8) {
                    Text("向 Edge 查詢當前的網路介面設定。")
                        .font(.footnote)
                        .foregroundColor(.secondary)
                    Button {
                        Task { await fetchNetworkConfig() }
                    } label: {
                        HStack {
                            if isLoading {
                                ProgressView()
                                    .progressViewStyle(.circular)
                            }
                            Text(isLoading ? "讀取中…" : "取得配置")
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

            Section("網路資訊") {
                if let config {
                    let dhcpDisplay: String? = {
                        guard config.mode != nil else { return nil }
                        return config.isDhcpEnabled ? "啟用" : "停用"
                    }()
                    InfoRow(title: "IP", value: config.ipAddress)
                    InfoRow(title: "Gateway", value: config.gateway)
                    InfoRow(title: "Subnet Mask", value: config.subnetMask)
                    InfoRow(title: "DHCP", value: dhcpDisplay)
                } else {
                    Text(isLoading ? "正在取得資料…" : "尚未取得資料，請點擊「取得配置」。")
                        .foregroundColor(.secondary)
                }
            }

            if let dns = config?.dns, !dns.isEmpty {
                Section("DNS") {
                    Text(dns)
                        .font(.subheadline.monospaced())
                        .multilineTextAlignment(.leading)
                }
            }
        }
        .navigationTitle("網路配置")
        .navigationBarTitleDisplayMode(.inline)
        .listStyle(.insetGrouped)
        .scrollContentBackground(.hidden)
        .background { AppBackground() }
        .listRowBackground(Color.surfaceBackground)
        .toolbar(.hidden, for: .tabBar)
    }

    private func fetchNetworkConfig() async {
        await MainActor.run {
            isLoading = true
            errorMessage = nil
        }
        do {
            let command = try await APIClient.shared.sendEdgeCommand(edgeId: edge.edgeId, code: "102")
            let result: EdgeCommandResultDTO<EdgeNetworkConfigDTO> = try await APIClient.shared.fetchEdgeCommandResult(traceId: command.traceId)
            await MainActor.run {
                self.config = result.result
                self.isLoading = false
                self.errorMessage = nil
            }
        } catch {
            await MainActor.run {
                self.config = nil
                self.isLoading = false
                self.errorMessage = error.localizedDescription
            }
        }
    }
}

private struct InfoRow: View {
    let title: String
    let value: String?

    var body: some View {
        HStack(alignment: .firstTextBaseline) {
            Text(title)
                .font(.subheadline.weight(.medium))
                .foregroundColor(.secondary)
                .frame(width: 110, alignment: .leading)
            Text(value?.isEmpty == false ? value! : "—")
                .font(.subheadline.monospaced())
                .foregroundColor(.primary)
        }
        .padding(.vertical, 4)
    }
}

#Preview {
    NavigationStack {
        NetworkConfigView(
            edge: EdgeSummary(edgeId: "RED-TEST0001", displayName: "測試 Edge", isOnline: true)
        )
    }
}
