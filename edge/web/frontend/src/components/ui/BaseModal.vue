<template>
  <Teleport to="body">
    <transition name="fade">
      <div
        v-if="open"
        class="fixed inset-0 z-50 flex items-center justify-center bg-black/30 px-4 py-8"
        role="dialog"
        :aria-modal="true"
        @keydown="handleKeydown"
        @click.self="handleBackdrop"
      >
        <div
          ref="dialogRef"
          class="w-full max-w-lg rounded-2xl bg-surface-default p-6 shadow-elev-lg focus:outline-none"
          tabindex="-1"
          @click.stop
        >
          <header class="mb-4 flex items-start justify-between gap-4">
            <div>
              <h2 class="text-lg font-semibold text-ink">{{ title }}</h2>
              <p v-if="description" class="text-sm text-ink-muted">{{ description }}</p>
            </div>
            <BaseButton
              variant="ghost"
              size="sm"
              type="button"
              :aria-label="closeButtonAriaLabel"
              @click="handleCloseClick"
            >
              ✕
            </BaseButton>
          </header>
          <div class="space-y-4">
            <slot />
          </div>
          <footer v-if="$slots.footer" class="mt-6 flex justify-end gap-3">
            <slot name="footer" />
          </footer>
        </div>
      </div>
    </transition>
  </Teleport>
</template>

<script setup lang="ts">
import { computed, onMounted, ref, watch } from 'vue';
import BaseButton from './BaseButton.vue';

/**
  * 組件用途：提供可 Teleport 的通用模態框，支援鍵盤關閉。
  * 輸入參數：open 決定顯示，title/description 作為標題欄資訊。
  * 與其他模組關聯：CameraBindModal、CameraPreviewModal 等皆依賴此容器。
  */

const props = defineProps<{
  open: boolean;
  title: string;
  description?: string;
  closeOnBackdrop?: boolean;
  closeOnEsc?: boolean;
  closeButtonAriaLabel?: string;
}>();

const emit = defineEmits(['close']);

const closeOnBackdrop = computed(() => props.closeOnBackdrop ?? true);
const closeOnEsc = computed(() => props.closeOnEsc ?? true);
const closeButtonAriaLabel = computed(() => props.closeButtonAriaLabel ?? '關閉');

const dialogRef = ref<HTMLDivElement>();

const focusableSelectors =
  'a[href], button:not([disabled]), textarea:not([disabled]), input:not([disabled]), select:not([disabled]), [tabindex]:not([tabindex="-1"])';

// 將焦點聚焦在對話框上，確保鍵盤操作體驗
const focusDialog = () => {
  dialogRef.value?.focus();
};

// 聚焦可互動元素，確保 Tab 循環在對話框內部
const trapFocus = (event: KeyboardEvent) => {
  const container = dialogRef.value;
  if (!container) {
    return;
  }
  const focusable = Array.from(container.querySelectorAll<HTMLElement>(focusableSelectors)).filter(
    (element) => !element.hasAttribute('disabled') && element.getAttribute('aria-hidden') !== 'true'
  );
  if (focusable.length === 0) {
    event.preventDefault();
    focusDialog();
    return;
  }
  const first = focusable[0];
  const last = focusable[focusable.length - 1];
  const current = document.activeElement as HTMLElement | null;

  if (event.shiftKey) {
    if (current === first || current === container) {
      event.preventDefault();
      last.focus();
    }
    return;
  }

  if (current === last) {
    event.preventDefault();
    first.focus();
  }
};

// 鍵盤事件：支援 Esc 關閉並導入 focus trap
const handleKeydown = (event: KeyboardEvent) => {
  if (event.key === 'Escape' && closeOnEsc.value) {
    event.preventDefault();
    emit('close');
    return;
  }
  if (event.key === 'Tab') {
    trapFocus(event);
  }
};

const requestClose = () => {
  if (!props.open) return;
  emit('close');
};

const handleCloseClick = () => {
  requestClose();
};

const handleBackdrop = () => {
  if (!closeOnBackdrop.value) {
    return;
  }
  requestClose();
};

onMounted(() => {
  if (props.open) {
    focusDialog();
  }
});

watch(
  () => props.open,
  (next) => {
    if (next) {
      focusDialog();
    }
  }
);
</script>

<style scoped>
.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.16s ease;
}

.fade-enter-from,
.fade-leave-to {
  opacity: 0;
}
</style>
