<template>
  <div class="flex min-h-screen bg-surface-subtle">
    <AppSidebar class="hidden md:flex" />
    <transition name="slide">
      <div v-if="uiStore.isSidebarOpen" class="fixed inset-0 z-40 flex md:hidden">
        <div class="flex-1 bg-black/40" @click="uiStore.toggleSidebar()" />
        <AppSidebar class="flex w-72" />
      </div>
    </transition>
    <div class="flex flex-1 flex-col">
      <AppTopBar />
      <main class="mx-auto w-full max-w-[1200px] flex-1 px-4 py-10">
        <RouterView />
      </main>
    </div>
    <BaseToast />
  </div>
</template>

<script setup lang="ts">
import { RouterView } from 'vue-router';
import AppSidebar from './AppSidebar.vue';
import AppTopBar from './AppTopBar.vue';
import BaseToast from '@/components/ui/BaseToast.vue';
import { useUiStore } from '@/store/ui';

/**
  * 組件用途：建立應用的主框架，整合側邊欄、頂部工具列與內容區。
  * 與其他模組關聯：透過 uiStore 控制行動側邊欄；BaseToast 顯示全域訊息。
  */

const uiStore = useUiStore();
</script>

<style scoped>
.slide-enter-active,
.slide-leave-active {
  transition: transform 0.2s ease, opacity 0.2s ease;
}
.slide-enter-from,
.slide-leave-to {
  transform: translateX(100%);
  opacity: 0;
}
</style>
