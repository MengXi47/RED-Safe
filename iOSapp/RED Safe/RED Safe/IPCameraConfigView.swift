import SwiftUI

struct IPCameraConfigView: View {
    let edge: EdgeSummary

    @State private var devices: [IPCameraDeviceDTO] = []
    @State private var addedDevices: [AddedIPCameraDTO] = []
    @State private var isLoading = false
    @State private var isFetchingAdded = false
    @State private var errorMessage: String?
    @State private var addedErrorMessage: String?
    @State private var lastTraceId: String?
    @State private var selectedDeviceForAdd: IPCameraDeviceDTO?
    @State private var addCustomName: String = ""
    @State private var addAccount: String = ""
    @State private var addPassword: String = ""
    @State private var addSensitivity: Int = 50
    @State private var addFormError: String?
    @State private var isSubmittingAdd = false
    @State private var addAlert: AddCameraAlert?
    @State private var deletingIP: String?

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

            if let message = addedErrorMessage {
                Section {
                    Label(message, systemImage: "exclamationmark.triangle.fill")
                        .foregroundColor(.orange)
                        .multilineTextAlignment(.leading)
                }
            }

            Section("掃描到的攝影機") {
                if devices.isEmpty {
                    Text(isLoading ? "正在取得資料…" : "尚未取得資料，請點擊「開始掃描」。")
                        .foregroundColor(.secondary)
                } else {
                    ForEach(devices) { device in
                        VStack(alignment: .leading, spacing: 6) {
                            HStack {
                                Label(device.name, systemImage: "camera.fill")
                                    .font(.headline)
                                Spacer()
                                if isDeviceAlreadyAdded(device) {
                                    Text("已新增")
                                        .font(.subheadline.weight(.semibold))
                                        .foregroundColor(.secondary)
                                        .padding(.horizontal, 12)
                                        .padding(.vertical, 6)
                                        .background(Color.secondary.opacity(0.15), in: Capsule())
                                } else {
                                    Button("新增") {
                                        openAddSheet(for: device)
                                    }
                                    .buttonStyle(.borderedProminent)
                                    .tint(.accentColor)
                                    .disabled(isSubmittingAdd || isDeletingAny)
                                }
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

            Section("已新增的攝影機") {
                if isFetchingAdded {
                    HStack {
                        Spacer()
                        ProgressView("正在載入資料…")
                            .progressViewStyle(.circular)
                        Spacer()
                    }
                    .padding(.vertical, 6)
                } else if addedDevices.isEmpty {
                    Text("尚未取得已新增的攝影機。")
                        .foregroundColor(.secondary)
                } else {
                    ForEach(addedDevices) { device in
                        VStack(alignment: .leading, spacing: 6) {
                            HStack {
                                Label(displayName(for: device), systemImage: "camera.fill")
                                    .font(.headline)
                                Spacer()
                                Button {
                                    Task { await removeCamera(device) }
                                } label: {
                                    if isDeleting(device) {
                                        ProgressView()
                                            .progressViewStyle(.circular)
                                    } else {
                                        Text("刪除")
                                    }
                                }
                                .buttonStyle(.borderedProminent)
                                .tint(.red)
                                .disabled(isDeletingAny)
                            }
                            VStack(alignment: .leading, spacing: 4) {
                                InfoRow(title: "IP", value: sanitized(device.ipAddress))
                                InfoRow(title: "MAC", value: sanitized(device.macAddress))
                            }
                        }
                        .padding(.vertical, 6)
                    }
                }
            }
        }
        .navigationTitle("IP Camera配置")
        .navigationBarTitleDisplayMode(.inline)
        .task(id: edge.edgeId) {
            await fetchAddedCameras()
        }
        .sheet(item: $selectedDeviceForAdd, onDismiss: resetAddForm) { device in
            NavigationStack {
                Form {
                    Section("攝影機資訊") {
                        InfoRow(title: "IPC", value: device.name)
                        InfoRow(title: "IP", value: device.ip)
                        InfoRow(title: "MAC", value: device.mac)
                    }
                    Section("新增設定") {
                        TextField("名稱 (自訂，可留空)", text: $addCustomName)
                            .textInputAutocapitalization(.none)
                            .autocorrectionDisabled()
                        TextField("帳號 (可留空)", text: $addAccount)
                            .textInputAutocapitalization(.none)
                            .autocorrectionDisabled()
                        SecureField("密碼 (可留空)", text: $addPassword)
                        Stepper(value: $addSensitivity, in: 0...100) {
                            HStack {
                                Text("靈敏度")
                                Spacer()
                                Text("\(addSensitivity)")
                                    .font(.body.monospacedDigit())
                                    .foregroundColor(.secondary)
                            }
                        }
                        if let message = addFormError, !message.isEmpty {
                            Text(message)
                                .font(.footnote)
                                .foregroundColor(.red)
                        }
                    }
                    if isSubmittingAdd {
                        Section {
                            HStack {
                                Spacer()
                                ProgressView("送出中…")
                                Spacer()
                            }
                        }
                    }
                }
                .navigationTitle("新增攝影機")
                .navigationBarTitleDisplayMode(.inline)
                .toolbar {
                    ToolbarItem(placement: .cancellationAction) {
                        Button("取消") {
                            selectedDeviceForAdd = nil
                        }
                        .disabled(isSubmittingAdd)
                    }
                    ToolbarItem(placement: .confirmationAction) {
                        Button(isSubmittingAdd ? "送出中…" : "確認") {
                            guard let device = selectedDeviceForAdd else { return }
                            Task { await submitAdd(for: device) }
                        }
                        .disabled(isSubmittingAdd)
                    }
                }
            }
        }
        .alert(item: $addAlert) { alert in
            Alert(
                title: Text(alert.title),
                message: Text(alert.message),
                dismissButton: .default(Text("好"))
            )
        }
    }

    private func triggerScan() async {
        await MainActor.run {
            isLoading = true
            errorMessage = nil
        }
        do {
            let command = try await APIClient.shared.sendEdgeCommand(edgeId: edge.edgeId, code: "101")
            let result: EdgeCommandResultDTO<[IPCameraDeviceDTO]> = try await APIClient.shared.fetchEdgeCommandResult(traceId: command.traceId)
            await MainActor.run {
                self.devices = result.result ?? []
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

    private func fetchAddedCameras() async {
        await MainActor.run {
            isFetchingAdded = true
            addedErrorMessage = nil
        }
        do {
            let command = try await APIClient.shared.sendEdgeCommand(edgeId: edge.edgeId, code: "103")
            let result: EdgeCommandResultDTO<[AddedIPCameraDTO]> = try await APIClient.shared.fetchEdgeCommandResult(traceId: command.traceId)
#if DEBUG
            print("📷 [IPCameraConfigView] Fetched added cameras count=\(result.result?.count ?? 0) traceId=\(result.traceId ?? command.traceId)")
#endif
            await MainActor.run {
                self.addedDevices = result.result ?? []
                self.isFetchingAdded = false
                self.addedErrorMessage = nil
            }
        } catch {
            await MainActor.run {
                self.isFetchingAdded = false
                self.addedErrorMessage = error.localizedDescription
            }
        }
    }

    private func openAddSheet(for device: IPCameraDeviceDTO) {
        addCustomName = ""
        addAccount = ""
        addPassword = ""
        addSensitivity = 50
        addFormError = nil
        isSubmittingAdd = false
        selectedDeviceForAdd = device
    }

    private func resetAddForm() {
        addCustomName = ""
        addAccount = ""
        addPassword = ""
        addSensitivity = 50
        addFormError = nil
        isSubmittingAdd = false
    }

    private func submitAdd(for device: IPCameraDeviceDTO) async {
        let trimmedName = addCustomName.trimmingCharacters(in: .whitespacesAndNewlines)
        let finalName = trimmedName.isEmpty ? device.name : trimmedName

        let payload = AddIPCameraCommandPayload(
            ip: device.ip,
            mac: device.mac,
            ipcName: device.name,
            customName: finalName,
            ipcAccount: addAccount.trimmingCharacters(in: .whitespacesAndNewlines),
            ipcPassword: addPassword,
            fallSensitivity: String(addSensitivity)
        )

        await MainActor.run {
            isSubmittingAdd = true
            addFormError = nil
        }

        do {
            let command = try await APIClient.shared.sendEdgeCommand(edgeId: edge.edgeId, code: "104", payload: payload)
            let result: EdgeCommandResultDTO<AddIPCameraResultDTO> = try await APIClient.shared.fetchEdgeCommandResult(traceId: command.traceId)

            let trimmedError = result.result?.errorMessage?.trimmingCharacters(in: .whitespacesAndNewlines)
            if result.status.lowercased() == "ok", (trimmedError?.isEmpty ?? true) {
                await MainActor.run {
                    isSubmittingAdd = false
                    selectedDeviceForAdd = nil
                    addAlert = AddCameraAlert(title: "新增成功", message: "攝影機已成功新增。")
                }
                await fetchAddedCameras()
            } else {
                let message = trimmedError?.isEmpty == false ? trimmedError! : "新增失敗，請稍後再試。"
                await MainActor.run {
                    isSubmittingAdd = false
                    addFormError = message
                }
            }
        } catch {
            await MainActor.run {
                isSubmittingAdd = false
                addFormError = error.localizedDescription
            }
        }
    }

    private func removeCamera(_ device: AddedIPCameraDTO) async {
        let normalized = normalizeIPAddress(device.ipAddress)
        await MainActor.run {
            deletingIP = normalized
            addedErrorMessage = nil
        }

        let payload = RemoveIPCameraCommandPayload(ip: device.ipAddress)

        do {
            let command = try await APIClient.shared.sendEdgeCommand(edgeId: edge.edgeId, code: "105", payload: payload)
            let result: EdgeCommandResultDTO<AddIPCameraResultDTO> = try await APIClient.shared.fetchEdgeCommandResult(traceId: command.traceId)

            let trimmedError = result.result?.errorMessage?.trimmingCharacters(in: .whitespacesAndNewlines)
            if result.status.lowercased() == "ok", (trimmedError?.isEmpty ?? true) {
                await MainActor.run {
                    deletingIP = nil
                    addAlert = AddCameraAlert(title: "刪除成功", message: "攝影機已成功刪除。")
                }
                await fetchAddedCameras()
            } else {
                let message = trimmedError?.isEmpty == false ? trimmedError! : "刪除失敗，請稍後再試。"
                await MainActor.run {
                    deletingIP = nil
                    addedErrorMessage = message
                }
            }
        } catch {
            await MainActor.run {
                deletingIP = nil
                addedErrorMessage = error.localizedDescription
            }
        }
    }

    private func displayName(for device: AddedIPCameraDTO) -> String {
        if !device.customName.isEmpty {
            return device.customName
        }
        if !device.ipcName.isEmpty {
            return device.ipcName
        }
        return device.ipAddress.isEmpty ? device.macAddress : device.ipAddress
    }

    private func sanitized(_ value: String) -> String {
        let trimmed = value.trimmingCharacters(in: .whitespacesAndNewlines)
        return trimmed.isEmpty ? "-" : trimmed
    }

    private var isDeletingAny: Bool {
        deletingIP != nil
    }

    private func isDeviceAlreadyAdded(_ device: IPCameraDeviceDTO) -> Bool {
        let deviceIP = normalizeIPAddress(device.ip)
        guard !deviceIP.isEmpty else { return false }
        return addedDevices.contains { normalizeIPAddress($0.ipAddress) == deviceIP }
    }

    private func isDeleting(_ device: AddedIPCameraDTO) -> Bool {
        guard let deletingIP else { return false }
        return normalizeIPAddress(device.ipAddress) == deletingIP
    }

    private func normalizeIPAddress(_ value: String) -> String {
        value.trimmingCharacters(in: .whitespacesAndNewlines)
            .lowercased()
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

private struct AddCameraAlert: Identifiable {
    let id = UUID()
    let title: String
    let message: String
}

#Preview {
    NavigationStack {
        IPCameraConfigView(edge: EdgeSummary(edgeId: "RED-TEST0001", displayName: "測試 Edge", isOnline: true))
    }
}
