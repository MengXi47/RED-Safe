import Foundation

/// HomeViewModel 專責處理裝置清單與相關操作，維持 UI 與網路層的清楚界線。
@MainActor
final class HomeViewModel: ObservableObject {
    @Published var edges: [EdgeSummary] = []
    @Published var isLoading: Bool = false
    @Published var message: String?
    @Published var showMessage: Bool = false

    /// 載入使用者綁定的 Edge 清單。
    func loadEdges(showIndicator: Bool = true) {
        Task { @MainActor in
            if showIndicator { isLoading = true }
            defer {
                if showIndicator { isLoading = false }
            }

            do {
                let fetched = try await APIClient.shared.fetchEdgeList()
                self.edges = sortEdges(fetched)
                if fetched.isEmpty {
                    self.showTempMessage("尚未綁定裝置，請新增裝置。")
                }
            } catch {
                self.showTempMessage(error.localizedDescription)
            }
        }
    }

    /// 嘗試綁定新 Edge 裝置。
    func bindEdge(edgeId: String, name: String, password: String) {
        Task { @MainActor in
            isLoading = true
            defer { isLoading = false }

            do {
                _ = try await APIClient.shared.bindEdge(edgeId: edgeId, displayName: name, edgePassword: password)
                showTempMessage("綁定成功")
                loadEdges(showIndicator: false)
            } catch {
                showTempMessage(error.localizedDescription)
            }
        }
    }

    /// 解除指定 Edge 的綁定。
    func unbindEdge(edgeId: String) {
        Task { @MainActor in
            isLoading = true
            defer { isLoading = false }

            do {
                _ = try await APIClient.shared.unbindEdge(edgeId: edgeId)
                showTempMessage("已解除綁定")
                edges.removeAll { $0.edgeId == edgeId }
            } catch {
                showTempMessage(error.localizedDescription)
            }
        }
    }

    /// 更新 Edge 顯示名稱。
    func renameEdge(edgeId: String, newName: String) {
        Task { @MainActor in
            isLoading = true
            defer { isLoading = false }

            do {
                _ = try await APIClient.shared.updateEdgeName(edgeId: edgeId, newName: newName)
                if let index = edges.firstIndex(where: { $0.edgeId == edgeId }) {
                    let edge = edges[index]
                    edges[index] = EdgeSummary(edgeId: edge.edgeId, displayName: newName, isOnline: edge.isOnline)
                    edges = sortEdges(edges)
                }
                showTempMessage("名稱已更新")
            } catch {
                showTempMessage(error.localizedDescription)
            }
        }
    }

    /// 更新 Edge 密碼。
    func updateEdgePassword(edgeId: String, currentPassword: String, newPassword: String) {
        Task { @MainActor in
            isLoading = true
            defer { isLoading = false }

            do {
                _ = try await APIClient.shared.updateEdgePassword(edgeId: edgeId, currentPassword: currentPassword, newPassword: newPassword)
                showTempMessage("Edge 密碼已更新")
            } catch {
                showTempMessage(error.localizedDescription)
            }
        }
    }

    /// 顯示短暫提示訊息。
    private func showTempMessage(_ text: String) {
        message = text
        showMessage = true
        DispatchQueue.main.asyncAfter(deadline: .now() + 2) { [weak self] in
            self?.showMessage = false
        }
    }

    private func normalizedName(for edge: EdgeSummary) -> String {
        let trimmed = edge.displayName.trimmingCharacters(in: .whitespacesAndNewlines)
        return trimmed.isEmpty ? edge.edgeId : trimmed
    }

    private func sortEdges(_ list: [EdgeSummary]) -> [EdgeSummary] {
        list.sorted { lhs, rhs in
            let leftName = normalizedName(for: lhs)
            let rightName = normalizedName(for: rhs)

            let comparison = leftName.localizedCaseInsensitiveCompare(rightName)
            if comparison == .orderedSame {
                return lhs.edgeId.localizedCaseInsensitiveCompare(rhs.edgeId) == .orderedAscending
            }
            return comparison == .orderedAscending
        }
    }
}
