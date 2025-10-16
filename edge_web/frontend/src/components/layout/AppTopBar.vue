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

const uiStore = useUiStore();
const route = useRoute();

const currentTitle = computed(() => String(route.meta.title ?? 'RED Safe'));

const toggleSidebar = () => uiStore.toggleSidebar();
</script>
