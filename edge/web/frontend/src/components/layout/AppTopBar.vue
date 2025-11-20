<template>
  <header class="sticky top-0 z-20 border-b border-border bg-surface-default/90 shadow-elev-sm backdrop-blur">
    <div class="mx-auto flex h-16 w-full max-w-[1200px] items-center justify-between px-4">
      <div class="flex items-center gap-3">
        <button class="md:hidden" @click="toggleSidebar">
          <span class="sr-only">切換選單</span>
          ☰
        </button>
        <slot name="title">
          <h1 class="text-lg font-semibold text-ink">{{ currentTitle }}</h1>
        </slot>
      </div>
      <div class="hidden items-center gap-3 md:flex"></div>
    </div>
  </header>
</template>

<script setup lang="ts">
import { useRoute } from 'vue-router';
import { computed } from 'vue';
import { useUiStore } from '@/store/ui';

/**
  * 組件用途：顯示頂部標題與行動裝置選單按鈕。
  * 與其他模組關聯：讀取路由 meta 標題並呼叫 uiStore 切換側邊欄。
  */

const uiStore = useUiStore();
const route = useRoute();

// 動態讀取 meta.title，若無則回到預設站名
const currentTitle = computed(() => String(route.meta.title ?? 'RED Safe'));

// 行動版菜單按鈕，切換側邊欄開啟狀態
const toggleSidebar = () => uiStore.toggleSidebar();
</script>
