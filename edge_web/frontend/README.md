# RED Safe Edge Web Frontend

Modernised Vue 3 + TypeScript SPA that powers the Edge web console.

## Getting Started

```bash
cd edge_web/frontend
npm install # or pnpm install / yarn install
npm run dev
```

The dev server defaults to `http://localhost:5173`. In Django, set `VITE_DEV_SERVER` in
`settings.py` (or environment) to proxy assets during development.

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
