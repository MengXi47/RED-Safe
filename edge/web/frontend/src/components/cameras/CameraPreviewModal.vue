<template>
  <BaseModal
    :open="open"
    :title="camera?.name ?? '攝影機預覽'"
    :description="camera?.ip"
    close-button-aria-label="Close preview"
    :close-on-backdrop="shouldCloseOnBackdrop"
    :close-on-esc="shouldCloseOnEsc"
    @close="onClose"
  >
    <div class="relative aspect-video w-full overflow-hidden rounded-2xl border border-border bg-black">
      <video ref="videoEl" autoplay muted playsinline class="h-full w-full object-contain" />
      <div
        v-if="loading"
        class="absolute inset-0 flex flex-col items-center justify-center gap-2 bg-black/60 text-white"
      >
        <BaseLoadingSpinner />
        <p class="text-sm">正在建立預覽...</p>
      </div>
      <div v-if="error" class="absolute inset-0 flex flex-col items-center justify-center gap-2 bg-danger/85 text-white">
        <p class="text-sm">{{ error }}</p>
      </div>
    </div>
  </BaseModal>
</template>

<script setup lang="ts">
import { computed, ref, watch, onBeforeUnmount } from 'vue';
import BaseModal from '@/components/ui/BaseModal.vue';
import BaseLoadingSpinner from '@/components/ui/BaseLoadingSpinner.vue';
import type { Camera } from '@/types/camera';

/**
  * 組件用途：顯示攝影機串流預覽並處理連線狀態提示。
  * 輸入參數：open 控制開啟、camera 提供標題資訊、sessionId 由父層管理。
  * 與其他模組關聯：CamerasView 透過 start/stop 事件建立與終止 WebRTC 連線。
  */

const props = defineProps<{
  open: boolean;
  camera: Camera | null;
  sessionId: string;
  closeOnBackdrop?: boolean;
  closeOnEsc?: boolean;
}>();

const emit = defineEmits<{
  (e: 'start', camera: Camera): void;
  (e: 'stop'): void;
  (e: 'close'): void;
}>();

const videoEl = ref<HTMLVideoElement>();
const loading = ref(false);
const error = ref('');
const shouldCloseOnBackdrop = computed(() => props.closeOnBackdrop ?? true);
const shouldCloseOnEsc = computed(() => props.closeOnEsc ?? true);

// 重置預覽狀態並清除串流，避免殘留畫面
const reset = () => {
  loading.value = false;
  error.value = '';
  if (videoEl.value) {
    videoEl.value.srcObject = null;
  }
};

// 關閉時通知外層停止串流，再行復位狀態
const onClose = () => {
  emit('stop');
  emit('close');
  reset();
};

const handleEscClose = (event: KeyboardEvent) => {
  if (!shouldCloseOnEsc.value || !props.open) {
    return;
  }
  if (event.key === 'Escape') {
    event.preventDefault();
    onClose();
  }
};

const syncEscListener = (shouldListen: boolean) => {
  if (typeof window === 'undefined') {
    return;
  }
  window.removeEventListener('keydown', handleEscClose);
  if (shouldListen) {
    window.addEventListener('keydown', handleEscClose);
  }
};

// 監聽開啟狀態，進行預覽初始化或清除
watch(
  () => props.open,
  (next) => {
    if (next && props.camera) {
      loading.value = true;
      error.value = '';
      emit('start', props.camera);
    } else {
      reset();
    }
  }
);

watch(
  () => shouldCloseOnEsc.value && props.open,
  (shouldListen) => {
    syncEscListener(shouldListen);
  },
  { immediate: true }
);

// 掛載即時串流並解除載入提示
const attachStream = (stream: MediaStream) => {
  if (videoEl.value && videoEl.value.srcObject !== stream) {
    videoEl.value.srcObject = stream;
    videoEl.value.play().catch(() => undefined);
  }
  loading.value = false;
};

// 預覽錯誤處理，顯示錯誤訊息於覆蓋層
const handleError = (message: string) => {
  loading.value = false;
  error.value = message;
};

defineExpose({ attachStream, handleError });

// 元件卸載時確保釋放串流資源
onBeforeUnmount(() => {
  reset();
  syncEscListener(false);
});
</script>
