<template>
  <div class="flex flex-col items-center justify-center rounded-2xl border border-dashed border-border bg-surface-subtle px-6 py-10 text-center">
    <div class="mb-4 inline-flex h-12 w-12 items-center justify-center rounded-full bg-brand-50 text-brand-600">
      <slot name="icon">
        <span v-if="icon === 'alert'">⚠️</span>
        <span v-else>📄</span>
      </slot>
    </div>
    <h3 class="text-base font-semibold text-ink">{{ title }}</h3>
    <p v-if="description" class="mt-2 text-sm text-ink-muted">{{ description }}</p>
    <div v-if="$slots.action" class="mt-4">
      <slot name="action" />
    </div>
  </div>
</template>

<script setup lang="ts">
/**
  * 組件用途：顯示無資料或待建置狀態的提示。
  * 輸入參數：title、description、icon 控制顯示文字與圖示。
  * 與其他模組關聯：設定頁、日誌頁與 BaseTable 無資料時共用。
  */

withDefaults(
  defineProps<{
    title: string;
    description?: string;
    icon?: 'document' | 'alert';
  }>(),
  {
    description: '',
    icon: 'document'
  }
);
</script>
