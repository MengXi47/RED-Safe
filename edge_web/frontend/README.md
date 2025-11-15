# RED Safe Edge Web Frontend

Modernised Vue 3 + TypeScript SPA that powers the Edge web console.

## Architecture Overview

- `src/App.vue`：根層容器，根據路由 meta 切換 `AuthLayout` 或主框架 `AppShell`。
- `src/components/layout/`：側邊欄、頂部列與主骨架，搭配 Pinia `ui` store 管理狀態。
- `src/components/ui/`：整理後的共用元件（按鈕、輸入框、表格、Toast 等）。
- `src/components/cameras/`：攝影機綁定與預覽專用模態。
- `src/lib/`：`http.ts` 與各項 `services` 封裝 API 呼叫；`useAuthThemeMeta` 處理登入頁主題色。
- `src/views/`：頁面級組件，保持 `<script setup>` 與 Composition API 實作。
- `src/store/ui.ts`：集中管理側邊欄與 Toast 佇列，供多個元件共用。

所有 Vue 元件與關鍵函式皆補齊中文註解，便於快速掌握用途與互動方式。

## Getting Started

```bash
cd edge_web/frontend
npm install # or pnpm install / yarn install
npm run dev
```

The dev server defaults to `http://localhost:5173`. In Django, set `VITE_DEV_SERVER` in
`settings.py` (or environment) to proxy assets during development.

### WebRTC Signaling Endpoint

- 即時顯示頁面會優先讀取 `VITE_FALL_WEBRTC_URL` 指向 signaling server。若未設定，則改用當前瀏覽器
  所在主機（`window.location.hostname`）的 `8765` 埠並發送到 `/webrtc/offer`，可透過 `VITE_FALL_WEBRTC_PORT`
  覆寫預設埠號。調整 `.env` 後重新建置即可。

## Production Build

```bash
npm run build
```

Compiled assets are emitted to `edge_web/static/frontend` together with a Vite
`manifest.json`. Django consumes the manifest via the `{% vite_scripts %}` template tag.

## Quality Gates

- `npm run lint` / `npm run lint:style`
- `npm run typecheck`
- `npm run test:unit`
- `npm run test:e2e`

Husky + lint-staged + commitlint are configured. Run `npm run prepare` after installing
dependencies to activate git hooks.

## Development Guidelines

- 使用 `<script setup>` 與 Composition API 撰寫元件，新增邏輯時同步補充中文註解。
- 透過 `@/lib/http.ts` 發出 API 請求，可自動帶入 CSRF Token 與錯誤處理。
- 若需新增共用元件，請放在 `src/components/ui/`，並保留與現有按鈕、輸入框一致的設計語彙。
- `npm run typecheck` 與 `npm run lint` 應於提交前通過，確保編譯與風格一致。
- 佈署前以 `npm run build` 驗證可成功輸出 `dist`（實際輸出在 `../static/frontend`）。
