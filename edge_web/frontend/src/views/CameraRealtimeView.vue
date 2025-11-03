<template>
  <div class="space-y-8">
    <header class="page-hero">
      <div class="flex flex-col gap-3 md:flex-row md:items-center md:justify-between">
        <div>
          <h2 class="text-2xl font-semibold text-ink">即時顯示</h2>
          <p class="mt-1 text-sm text-ink-muted">即時監看攝影機畫面的功能尚在規劃。</p>
        </div>
      </div>
    </header>

    <BaseCard title="即時串流預覽" description="待完成的串流框架預留在此區塊。">
      <div
        class="flex h-72 items-center justify-center rounded-xl border border-dashed border-border bg-surface-subtle text-ink-muted"
      >
        待實作
      </div>
    </BaseCard>
  </div>
</template>

<script setup lang="ts">
import BaseCard from '@/components/ui/BaseCard.vue';

/**
  * 組件用途：攝影機即時顯示頁面的骨架，等待後續串流功能補齊。
  * 與其他模組關聯：預計串接 WebRTC 或 RTSP 串流元件後顯示於 BaseCard 內。
  */
</script>
