# 重建說明

## 專案結構與責任分層
- `src/composables/`：封裝跨元件共享的操作流程，例如攝影機掃描、綁定與預覽邏輯。
- `src/lib/`：提供網路層與公用 hook，包括 Axios 客戶端、初始狀態擷取等工具。
- `src/components/ui/`：保留可重用的基礎 UI 元件，統一處理樣式、無障礙屬性與互動狀態。
- `src/views/`：以組合式 API 建立頁面，負責資料綁定與呼叫對應的 service/composable。
- `src/types/`：集中管理跨模組共用型別，確保 service 與畫面均使用顯式型別。

## 主要設計決策
1. **Axios 攔截器與錯誤統一格式**：在 `lib/http.ts` 建立 Axios 實例，集中處理 CSRF、逾時與重試；所有服務函式皆回傳一致的 `HttpError`。
2. **UI 狀態集中管理**：`store/ui.ts` 新增側邊欄顯示、Toast、全域 Loading 堆疊等行為，方便元件以 action 控制 UI 狀態。
3. **路由守衛註解化**：`router/index.ts` 以 meta 描述佈局與授權條件，並於 beforeEach 留下待串接後端驗證的掛載點。

## 移除檔案清單與理由
- `components/ui/BaseCheckbox.vue`
- `components/ui/BaseDrawer.vue`
- `components/ui/BaseSelect.vue`
- `components/ui/BaseSwitch.vue`
- `components/ui/BaseTabs.vue`
- `components/ui/BaseTooltip.vue`

> 上述元件均未在現行頁面或服務中引用，保留會增加維護負擔，故移除。

## 效能與維護性優化
- 導入 `useInitialState` 以型別安全地還原 SSR 資料，避免重複的 `window` 解析程式碼。
- 以 `useCameraManagement` 將掃描、綁定、預覽分層，降低 `CamerasView` 內部複雜度。
- `MetricProgress` 透過 computed 緩存條寬與百分比，避免高頻渲染時的重算。
- `BaseModal` 加入鍵盤 focus trap 與 Esc 關閉註釋，改善可近用性並統一行為。
- 全域樣式補充層級說明，協助新成員快速理解 Reset/Base/Utilities 分工。

## 驗證
- `npm run lint`
