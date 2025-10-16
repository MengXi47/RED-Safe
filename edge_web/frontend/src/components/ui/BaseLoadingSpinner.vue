<template>
  <svg
    :class="spinnerClass"
    viewBox="0 0 24 24"
    role="status"
    aria-live="polite"
    aria-busy="true"
  >
    <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4" />
    <path
      class="opacity-75"
      fill="currentColor"
      d="M4 12a8 8 0 018-8v4a4 4 0 00-4 4H4z"
    />
  </svg>
</template>

<script setup lang="ts">
import { computed } from 'vue';

type Size = 'sm' | 'md' | 'lg';

const props = withDefaults(
  defineProps<{
    size?: Size;
  }>(),
  {
    size: 'md'
  }
);

const spinnerClass = computed(() => {
  const base = 'animate-spin text-brand-500';
  const sizeMap = {
    sm: 'h-4 w-4',
    md: 'h-6 w-6',
    lg: 'h-10 w-10'
  };
  return `${base} ${sizeMap[props.size]}`;
});
</script>
