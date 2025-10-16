<template>
  <Teleport to="body">
    <transition name="fade">
      <div
        v-if="open"
        class="fixed inset-0 z-50 flex items-center justify-center bg-black/30 px-4 py-8"
        role="dialog"
        :aria-modal="true"
        @keydown.esc.prevent="$emit('close')"
      >
        <div
          ref="dialogRef"
          class="w-full max-w-lg rounded-2xl bg-white p-6 shadow-soft focus:outline-none"
          tabindex="-1"
        >
          <header class="mb-4 flex items-start justify-between gap-4">
            <div>
              <h2 class="text-lg font-semibold text-ink">{{ title }}</h2>
              <p v-if="description" class="text-sm text-ink-muted">{{ description }}</p>
            </div>
            <BaseButton variant="ghost" size="sm" aria-label="關閉" @click="$emit('close')">
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
import { onMounted, ref, watch } from 'vue';
import BaseButton from './BaseButton.vue';

const props = defineProps<{
  open: boolean;
  title: string;
  description?: string;
}>();

defineEmits(['close']);

const dialogRef = ref<HTMLDivElement>();

const focusDialog = () => {
  dialogRef.value?.focus();
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
