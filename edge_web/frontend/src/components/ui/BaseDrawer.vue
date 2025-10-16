<template>
  <Teleport to="body">
    <transition name="drawer">
      <div
        v-if="open"
        class="fixed inset-0 z-40 flex"
        role="dialog"
        :aria-modal="true"
        @keydown.esc.prevent="$emit('close')"
      >
        <div class="flex-1 bg-black/30" @click="$emit('close')" />
        <aside
          class="relative h-full w-full max-w-md border-l border-border bg-surface-default px-6 py-8 shadow-elev-lg focus:outline-none"
          tabindex="-1"
          ref="panel"
        >
          <header class="mb-6 flex items-start justify-between gap-4">
            <div>
              <h2 class="text-lg font-semibold text-ink">{{ title }}</h2>
              <p v-if="description" class="text-sm text-ink-muted">{{ description }}</p>
            </div>
            <BaseButton variant="ghost" size="sm" aria-label="關閉" @click="$emit('close')">
              ✕
            </BaseButton>
          </header>
          <div class="flex flex-col gap-4">
            <slot />
          </div>
          <footer v-if="$slots.footer" class="mt-8 flex justify-end gap-3">
            <slot name="footer" />
          </footer>
        </aside>
      </div>
    </transition>
  </Teleport>
</template>

<script setup lang="ts">
import { ref, watch } from 'vue';
import BaseButton from './BaseButton.vue';

const props = defineProps<{
  open: boolean;
  title: string;
  description?: string;
}>();

defineEmits(['close']);

const panel = ref<HTMLElement>();

watch(
  () => props.open,
  (next) => {
    if (next) {
      panel.value?.focus();
    }
  }
);
</script>

<style scoped>
.drawer-enter-active,
.drawer-leave-active {
  transition: transform 0.2s ease, opacity 0.2s ease;
}

.drawer-enter-from,
.drawer-leave-to {
  transform: translateX(100%);
  opacity: 0;
}
</style>
