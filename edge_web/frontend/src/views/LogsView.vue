<template>
  <div class="space-y-8">
    <header class="page-hero">
      <h2 class="text-2xl font-semibold text-ink">系統日誌</h2>
      <p class="mt-1 text-sm text-ink-muted">此頁面將整合系統事件紀錄與匯出功能。</p>
    </header>
    <BaseEmpty title="尚未整合" description="後續版本將補上系統日誌視覺化。" />
  </div>
</template>

<script setup lang="ts">
import BaseEmpty from '@/components/ui/BaseEmpty.vue';
</script>
