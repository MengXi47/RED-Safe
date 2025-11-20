<template>
  <section class="rounded-3xl border border-border bg-surface-default p-6 shadow-elev-sm">
    <header v-if="title || $slots.header" class="mb-4 flex items-start justify-between gap-4">
      <div>
        <h2 v-if="title" class="text-lg font-semibold text-ink">{{ title }}</h2>
        <p v-if="description" class="text-sm text-ink-muted">{{ description }}</p>
      </div>
      <slot name="header" />
    </header>
    <div class="space-y-4">
      <slot />
    </div>
    <footer v-if="$slots.footer" class="mt-6">
      <slot name="footer" />
    </footer>
  </section>
</template>

<script setup lang="ts">
/**
  * 組件用途：提供統一的卡片樣式容器，可放置標題、描述與自訂區塊。
  * 輸入參數：title、description 控制標題文字，另外接受 header/footer slot。
  * 與其他模組關聯：各頁資訊面板共用此元件維持視覺一致。
  */

defineProps<{
  title?: string;
  description?: string;
}>();
</script>
