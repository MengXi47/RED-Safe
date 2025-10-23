import { defineStore } from 'pinia';
import { ref } from 'vue';
import {
  applyThemePreference,
  persistPreference,
  resolveEffectiveTheme,
  resolveThemePreference,
  subscribeSystemTheme,
  type ThemeMode,
  type ThemePreference
} from '@/lib/themePreference';

/**
  * Store 用途：集中管理使用者的主題偏好，支援儲存、系統同步與即時套用。
  * 與其他模組關聯：SettingsView 透過此 store 切換模式，整個儀表板的樣式相應更新。
  */

export const useThemeStore = defineStore('theme', () => {
  const preference = ref<ThemePreference>(resolveThemePreference());
  const effectiveTheme = ref<ThemeMode>(resolveEffectiveTheme(preference.value));
  let tearDownSystemListener: (() => void) | null = null;

  const stopSystemListener = () => {
    if (tearDownSystemListener) {
      tearDownSystemListener();
      tearDownSystemListener = null;
    }
  };

  const startSystemListener = () => {
    stopSystemListener();
    tearDownSystemListener = subscribeSystemTheme((mode) => {
      if (preference.value !== 'system') {
        stopSystemListener();
        return;
      }
      effectiveTheme.value = mode;
    });
  };

  const applyPreference = (next: ThemePreference, { persist = true } = {}) => {
    applyThemePreference(next);
    if (persist) {
      persistPreference(next);
    }
    effectiveTheme.value = resolveEffectiveTheme(next);

    if (next === 'system') {
      startSystemListener();
    } else {
      stopSystemListener();
    }
  };

  const setPreference = (next: ThemePreference) => {
    preference.value = next;
    applyPreference(next);
  };

  const toggleTheme = () => {
    const next = preference.value === 'dark' ? 'light' : 'dark';
    setPreference(next);
  };

  const hydrate = (initialPreference: ThemePreference) => {
    preference.value = initialPreference;
    applyPreference(initialPreference, { persist: false });
  };

  // 初始化時套用偏好，以處理未呼叫 hydrate 的情境
  if (typeof window !== 'undefined') {
    applyPreference(preference.value, { persist: false });
  }

  return {
    preference,
    theme: effectiveTheme,
    setPreference,
    toggleTheme,
    hydrate
  };
});
