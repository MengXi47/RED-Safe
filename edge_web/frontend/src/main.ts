import { createApp } from 'vue';
import { createPinia } from 'pinia';
import App from './App.vue';
import router from './router';
import './styles/global.css';
import type { EdgeInitialState } from '@/types/bootstrap';
import { bootstrapTheme } from '@/lib/themePreference';
import { useThemeStore } from '@/store/theme';

/**
  * 檔案用途：初始化 Vue 應用、還原伺服器注入的狀態並掛載全域插件。
  * 與其他模組關聯：注入 Pinia 與 Router，讓所有元件取得狀態與路由資訊。
  */

const initialElement = document.getElementById('edge-initial-state');
if (initialElement?.textContent) {
  try {
    const parsed = JSON.parse(initialElement.textContent) as EdgeInitialState;
    window.__EDGE_INITIAL_STATE__ = parsed;
  } catch (error) {
    console.warn('無法解析伺服器注入的初始狀態', error);
  }
}

const app = createApp(App);

const initialPreference = bootstrapTheme();

const pinia = createPinia();

app.use(pinia);
app.use(router);

const themeStore = useThemeStore(pinia);
themeStore.hydrate(initialPreference);

app.mount('#app');
