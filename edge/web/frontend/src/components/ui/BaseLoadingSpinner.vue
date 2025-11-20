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

/**
  * 組件用途：提供全站一致的載入動畫 SVG。
  * 輸入參數：size 控制尺寸，可搭配按鈕或區塊使用。
  * 與其他模組關聯：BaseButton、CameraPreviewModal 等載入狀態皆引用。
  */

type Size = 'sm' | 'md' | 'lg';

const props = withDefaults(
  defineProps<{
    size?: Size;
  }>(),
  {
    size: 'md'
  }
);

// 選擇不同尺寸對應的 class，並套用基礎旋轉動畫
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
