<template>
  <div class="space-y-4">
    <div class="flex items-baseline justify-between">
      <p class="text-3xl font-semibold text-ink">{{ Math.round(value) }}%</p>
      <p class="text-sm text-ink-muted">{{ label }}</p>
    </div>
    <div class="h-3 w-full overflow-hidden rounded-full bg-slate-100">
      <div
        class="h-full rounded-full bg-gradient-to-r from-brand-500 to-brand-600 transition-all"
        :style="{ width: `${Math.min(value, 100)}%` }"
      />
    </div>
  </div>
</template>

<script setup lang="ts">
defineProps<{
  value: number;
  label: string;
}>();
</script>
