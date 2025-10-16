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
