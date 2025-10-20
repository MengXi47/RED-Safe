<template>
  <div class="space-y-4">
    <div class="flex items-baseline justify-between">
      <p class="text-3xl font-semibold text-ink">{{ roundedValue }}%</p>
      <p class="text-sm text-ink-muted">{{ label }}</p>
    </div>
    <div class="h-3 w-full overflow-hidden rounded-full bg-surface-subtle">
      <div
        class="h-full rounded-full bg-gradient-to-r from-brand-500 to-brand-600 transition-all"
        :style="{ width: barWidth }"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue';

/**
  * 組件用途：以百分比條呈現儀表板指標數據。
  * 輸入參數：value 代表百分比、label 說明數據內容。
  * 與其他模組關聯：DashboardView 各項資源監控皆使用此視覺化。
  */
const props = defineProps<{
  value: number;
  label: string;
}>();

// 使用 computed 降低重繪次數，當 value 未變更時避免重算條寬
const clampedValue = computed(() => Math.min(Math.max(Number.isFinite(props.value) ? props.value : 0, 0), 100));
const roundedValue = computed(() => Math.round(clampedValue.value));
const barWidth = computed(() => `${clampedValue.value}%`);
</script>
