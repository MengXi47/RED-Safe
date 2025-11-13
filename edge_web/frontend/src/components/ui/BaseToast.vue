<template>
  <Teleport to="body">
    <div class="fixed right-6 top-6 z-[999] flex max-w-sm flex-col gap-3" role="region" aria-live="polite">
      <transition-group name="toast">
        <div
          v-for="toast in toasts"
          :key="toast.id"
          :class="toastClass(toast.variant)"
          role="alert"
          aria-live="assertive"
        >
          <div class="flex items-center justify-between gap-3">
            <div>
              <p class="text-sm font-medium text-white">{{ toast.message }}</p>
            </div>
            <button type="button" class="text-white/80" aria-label="關閉通知" @click="dismiss(toast.id)">✕</button>
          </div>
        </div>
      </transition-group>
    </div>
  </Teleport>
</template>

<script setup lang="ts">
import { storeToRefs } from 'pinia';
import { useUiStore } from '@/store/ui';

/**
  * 組件用途：集中顯示 Toast 通知，使用 Teleport 渲染在視窗右上角。
  * 與其他模組關聯：透過 uiStore 的 toastQueue 與 dismissToast 控制通知生命週期。
  */

const uiStore = useUiStore();
const { toastQueue } = storeToRefs(uiStore);
const { dismissToast } = uiStore;

const toasts = toastQueue;

// 根據通知種類回傳對應底色樣式
const toastClass = (variant: 'success' | 'danger' | 'info' = 'info') => {
  /**
   * Tailwind 無法為以 CSS variables 定義的色票生成 `/90` 透明度類別，
   * 造成淺色主題下背景幾乎透明。改用實色搭配 `bg-opacity-90` 確保對比。
   */
  const base = 'w-full rounded-2xl px-4 py-3 shadow-elev-lg backdrop-blur-md text-white bg-opacity-90';
  const map = {
    info: 'bg-brand-600',
    success: 'bg-success',
    danger: 'bg-danger'
  };
  return `${base} ${map[variant]}`;
};

// 代理關閉事件，呼叫 store 移除指定 Toast
const dismiss = (id: number) => {
  dismissToast(id);
};
</script>

<style scoped>
.toast-enter-active,
.toast-leave-active {
  transition: all 0.18s ease;
}

.toast-enter-from,
.toast-leave-to {
  opacity: 0;
  transform: translateY(-10px) scale(0.96);
}
</style>
