# RED‑Safe Web UI (Glassmorphism Redesign)

現代化重構：採用極簡 + 玻璃擬態（Glassmorphism）風格並提供深淺色雙主題。

## 技術棧
- SvelteKit (^2) + Vite
- 自訂設計系統（`src/lib/styles/design-system.css`）
- 主題 Store（`src/lib/stores/theme.js`）持久化於 `localStorage`
- 可組合元件：`SectionHeader`, `StatGrid`, `Footer`, `SignHeader`。

## 快速啟動
```sh
npm install
npm run dev
```
瀏覽器自動載入，使用導覽列右側按鈕切換主題。

## 設計系統概念
CSS 變數集中於 `:root` 與 `[data-theme="dark"]`：
- 色彩：`--color-bg`, `--color-surface`, `--color-accent` 等。
- 半徑：`--radius-sm`~`--radius-xl`；陰影層級 `--shadow-sm/md/lg`。
- 排版：類別 `.h1 .h2 .h3 .lead .muted`。
- 工具：`.container`, `.grid.auto-fit-*`, `.stack`, `.glass`, `.card`。

## 主題切換
`theme` store 訂閱後自動更新 `document.documentElement.dataset.theme`。
```js
import { theme } from '$lib/stores/theme';
theme.set('dark'); // 或 'light'
```

## 可及性（Accessibility）
- 使用語意標籤：`<nav> <main> <section> <footer>`。
- 列表區塊加上 `role="list"` 與 `role="listitem"`。
- 焦點樣式：`:focus-visible` 顯示主題色外框。

## 檔案導覽
```
src/
	lib/
		styles/design-system.css   # 全域設計系統與工具類
		stores/theme.js            # 深淺色主題持久化
		components/                # UI 元件
			SignHeader.svelte        # 玻璃導覽列 + 主題切換
			SectionHeader.svelte     # 區段標題組件
			StatGrid.svelte          # 指標卡片網格
			Footer.svelte            # 頁尾
	routes/
		+layout.svelte             # 匯入設計系統與導覽列
		+page.svelte               # 首頁（已重構為設計系統樣式）
		about/+page.svelte         # 產品藍圖頁
```

## 後續工作建議
- 將首頁模組化區塊再拆分成獨立元件（CapabilityGrid、ModuleDeck）。
- 加入表單驗證 / 互動回饋（Toast、Modal）。
- Lighthouse / Axe 檢測擴充可及性。

## 部署
建置：
```sh
npm run build
```
預覽：
```sh
npm run preview
```
若需特定平台請安裝對應 Adapter（如 `@sveltejs/adapter-vercel`）。

## 授權
（依原專案政策；若需另行規範請補充）。
