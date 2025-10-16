import { createApp } from 'vue';
import { createPinia } from 'pinia';
import App from './App.vue';
import router from './router';
import './styles/global.css';

const initialElement = document.getElementById('edge-initial-state');
if (initialElement?.textContent) {
  try {
    (window as Window & { __EDGE_INITIAL_STATE__?: any }).__EDGE_INITIAL_STATE__ = JSON.parse(
      initialElement.textContent,
    );
  } catch (error) {
    console.warn('Failed to parse initial state', error);
  }
}

const app = createApp(App);

app.use(createPinia());
app.use(router);

app.mount('#app');
