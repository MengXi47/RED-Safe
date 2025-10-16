<template>
  <span class="relative inline-flex" @mouseenter="show" @mouseleave="hide" @focus="show" @blur="hide">
    <slot />
    <transition name="fade">
      <span
        v-if="visible"
        class="absolute bottom-full mb-2 w-max max-w-xs rounded-md bg-ink text-xs text-white shadow-elev-sm"
        role="tooltip"
      >
        <span class="block px-2 py-1">{{ text }}</span>
      </span>
    </transition>
  </span>
</template>

<script setup lang="ts">
import { ref } from 'vue';

const props = defineProps<{
  text: string;
}>();

const visible = ref(false);

const show = () => {
  visible.value = true;
};
const hide = () => {
  visible.value = false;
};
</script>

<style scoped>
.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.12s ease;
}
.fade-enter-from,
.fade-leave-to {
  opacity: 0;
}
</style>
