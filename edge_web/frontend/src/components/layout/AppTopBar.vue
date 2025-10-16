<template>
  <header class="sticky top-0 z-20 border-b border-slate-200 bg-surface-default/95 backdrop-blur">
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
      <div class="flex items-center gap-3">
        <BaseTooltip text="重新整理">
          <BaseButton variant="ghost" size="sm" @click="refreshPage">
            🔄
          </BaseButton>
        </BaseTooltip>
      </div>
    </div>
  </header>
</template>

<script setup lang="ts">
import { useRoute } from 'vue-router';
import { computed } from 'vue';
import { useUiStore } from '@/store/ui';
import BaseButton from '@/components/ui/BaseButton.vue';
import BaseTooltip from '@/components/ui/BaseTooltip.vue';

const uiStore = useUiStore();
const route = useRoute();

const currentTitle = computed(() => String(route.meta.title ?? 'RED Safe'));

const toggleSidebar = () => uiStore.toggleSidebar();
const refreshPage = () => window.location.reload();
</script>
