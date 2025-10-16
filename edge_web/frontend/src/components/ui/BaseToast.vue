<template>
  <Teleport to="body">
    <div class="fixed right-6 top-6 z-[999] flex max-w-sm flex-col gap-3">
      <transition-group name="toast">
        <div
          v-for="toast in toasts"
          :key="toast.id"
          :class="toastClass(toast.variant)"
        >
          <div class="flex items-start justify-between gap-3">
            <div>
              <p class="text-sm font-medium text-white">{{ toast.message }}</p>
            </div>
            <button type="button" class="text-white/80" @click="dismiss(toast.id)">✕</button>
          </div>
        </div>
      </transition-group>
    </div>
  </Teleport>
</template>

<script setup lang="ts">
import { storeToRefs } from 'pinia';
import { useUiStore } from '@/store/ui';

const uiStore = useUiStore();
const { toastQueue } = storeToRefs(uiStore);
const { dismissToast } = uiStore;

const toasts = toastQueue;

const toastClass = (variant: 'success' | 'danger' | 'info' = 'info') => {
  const base = 'w-full rounded-2xl px-4 py-3 shadow-lg backdrop-blur-md text-white';
  const map = {
    info: 'bg-brand-600/95',
    success: 'bg-green-600/95',
    danger: 'bg-red-600/95'
  };
  return `${base} ${map[variant]}`;
};

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
