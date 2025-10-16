<template>
  <div class="space-y-8">
    <header>
      <h2 class="text-2xl font-semibold text-ink">系統設定</h2>
      <p class="mt-1 text-sm text-ink-muted">集中管理 Edge 裝置的系統參數與整合設定。</p>
    </header>
    <BaseEmpty title="尚待建置" description="可在此整合遠端服務、通知與時間同步等設定。" />
  </div>
</template>

<script setup lang="ts">
import BaseEmpty from '@/components/ui/BaseEmpty.vue';
</script>
