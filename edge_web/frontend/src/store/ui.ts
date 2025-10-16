import { defineStore } from 'pinia';
import { ref } from 'vue';

export const useUiStore = defineStore('ui', () => {
  const isSidebarOpen = ref(false);
  const toastQueue = ref<{ id: number; message: string; variant?: 'success' | 'danger' | 'info' }[]>([]);
  let toastId = 0;

  const toggleSidebar = () => {
    isSidebarOpen.value = !isSidebarOpen.value;
  };

  const pushToast = (message: string, variant: 'success' | 'danger' | 'info' = 'info') => {
    toastQueue.value.push({ id: ++toastId, message, variant });
    setTimeout(() => dismissToast(toastId), 4000);
  };

  const dismissToast = (id: number) => {
    toastQueue.value = toastQueue.value.filter((toast) => toast.id !== id);
  };

  return { isSidebarOpen, toggleSidebar, toastQueue, pushToast, dismissToast };
});
