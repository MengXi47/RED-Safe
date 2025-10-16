import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";
import path from "node:path";

export default defineConfig({
  plugins: [vue()],
  resolve: {
    alias: {
      "@": path.resolve(__dirname, "src")
    }
  },
  css: {
    postcss: path.resolve(__dirname, "postcss.config.cjs")
  },
  build: {
    outDir: path.resolve(__dirname, "../static/frontend"),
    emptyOutDir: true,
    manifest: true,
    rollupOptions: {
      input: path.resolve(__dirname, "src/main.ts")
    }
  },
  server: {
    port: 5173,
    strictPort: true,
    host: "0.0.0.0"
  }
});
