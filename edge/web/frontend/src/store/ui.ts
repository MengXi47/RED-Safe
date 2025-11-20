import { defineStore } from 'pinia';
import { computed, ref } from 'vue';

/**
  * Store 用途：集中管理側邊欄開合狀態與 Toast 佇列，供多個元件共用。
  * 與其他模組關聯：AppShell、BaseToast 等 UI 元件會監聽這些狀態。
  */

export const useUiStore = defineStore('ui', () => {
  // 側邊欄顯示狀態：AppShell/AppTopBar 會依據此值切換行動裝置選單
  const isSidebarOpen = ref(false);
  // Toast 佇列：BaseToast 逐筆渲染，維持通知顯示順序
  const toastQueue = ref<{ id: number; message: string; variant?: 'success' | 'danger' | 'info' }[]>([]);
  // 全域載入堆疊：同時有多處 loading 時避免互相蓋掉
  const activeLoadings = ref(new Set<string>());
  let toastId = 0;

  // 開啟側邊欄，主要提供外層直接指定顯示狀態
  const openSidebar = () => {
    isSidebarOpen.value = true;
  };

  // 關閉側邊欄，避免外層還需判斷目前狀態
  const closeSidebar = () => {
    isSidebarOpen.value = false;
  };

  // 切換側邊欄的可見狀態（行動裝置使用）
  const toggleSidebar = () => {
    isSidebarOpen.value = !isSidebarOpen.value;
  };

  // 推入 Toast 並預設四秒後自動關閉
  const pushToast = (message: string, variant: 'success' | 'danger' | 'info' = 'info') => {
    const id = ++toastId;
    toastQueue.value.push({ id, message, variant });
    window.setTimeout(() => dismissToast(id), 4000);
  };

  // 移除指定 Toast，供自動與手動關閉共用
  const dismissToast = (id: number) => {
    toastQueue.value = toastQueue.value.filter((toast) => toast.id !== id);
  };

  // 記錄 loading 來源，避免不同模組互相覆寫狀態
  const beginLoading = (key: string) => {
    const next = new Set(activeLoadings.value);
    next.add(key);
    activeLoadings.value = next;
  };

  // 結束指定 loading，若找不到 key 則忽略
  const endLoading = (key: string) => {
    if (!activeLoadings.value.has(key)) return;
    const next = new Set(activeLoadings.value);
    next.delete(key);
    activeLoadings.value = next;
  };

  // 清空所有 loading 狀態，避免例外情境殘留
  const resetLoading = () => {
    if (activeLoadings.value.size === 0) return;
    activeLoadings.value = new Set();
  };

  // 是否存在任一 loading，供全域遮罩或指標使用
  const isGlobalLoading = computed(() => activeLoadings.value.size > 0);

  return {
    isSidebarOpen,
    openSidebar,
    closeSidebar,
    toggleSidebar,
    toastQueue,
    pushToast,
    dismissToast,
    beginLoading,
    endLoading,
    resetLoading,
    isGlobalLoading
  };
});
