<template>
  <BaseModal :open="open" :title="camera?.name ?? '攝影機預覽'" :description="camera?.ip" @close="onClose">
    <div class="aspect-video w-full overflow-hidden rounded-2xl border border-border bg-black">
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
import { ref, watch, onBeforeUnmount } from 'vue';
import BaseModal from '@/components/ui/BaseModal.vue';
import BaseLoadingSpinner from '@/components/ui/BaseLoadingSpinner.vue';
import type { Camera } from '@/types/camera';

const props = defineProps<{
  open: boolean;
  camera: Camera | null;
  sessionId: string;
}>();

const emit = defineEmits<{
  (e: 'start', camera: Camera): void;
  (e: 'stop'): void;
  (e: 'close'): void;
}>();

const videoEl = ref<HTMLVideoElement>();
const loading = ref(false);
const error = ref('');

const reset = () => {
  loading.value = false;
  error.value = '';
  if (videoEl.value) {
    videoEl.value.srcObject = null;
  }
};

const onClose = () => {
  emit('stop');
  emit('close');
  reset();
};

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

const attachStream = (stream: MediaStream) => {
  if (videoEl.value && videoEl.value.srcObject !== stream) {
    videoEl.value.srcObject = stream;
    videoEl.value.play().catch(() => undefined);
  }
  loading.value = false;
};

const handleError = (message: string) => {
  loading.value = false;
  error.value = message;
};

defineExpose({ attachStream, handleError });

onBeforeUnmount(() => {
  reset();
});
</script>
