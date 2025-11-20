<template>
  <div class="space-y-8">
    <header class="page-hero">
      <h2 class="text-2xl font-semibold text-ink">系統設定</h2>
    </header>
    <BaseCard title="">
      <div class="flex flex-col gap-3 sm:flex-row sm:items-center sm:justify-between">
        <div class="space-y-1">
          <h3 class="text-base font-semibold text-ink">顯示模式</h3>
        </div>
        <div class="theme-toggle" role="radiogroup" aria-label="主題選擇">
          <span class="theme-toggle__indicator" :style="indicatorStyle" aria-hidden="true" />
          <button
            v-for="option in themeOptions"
            :key="option.value"
            type="button"
            class="theme-toggle__option"
            :class="{ 'theme-toggle__option--active': preference === option.value }"
            role="radio"
            :aria-checked="preference === option.value"
            @click="selectTheme(option.value)"
          >
            {{ option.label }}
          </button>
        </div>
      </div>
    </BaseCard>
    <BaseEmpty title="更多設定即將到來" description="未來將整合遠端服務、通知與時間同步等進階選項。" />
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue';
import BaseEmpty from '@/components/ui/BaseEmpty.vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import { useThemeStore } from '@/store/theme';
import type { ThemePreference } from '@/lib/themePreference';

/**
  * 組件用途：提供系統設定的入口，並加入主題切換功能。
  * 與其他模組關聯：透過 theme store 讓整個儀表板即時套用深淺色偏好。
  */

const themeStore = useThemeStore();

const themeOptions = [
  { label: '淺色', value: 'light' as ThemePreference },
  { label: '依系統', value: 'system' as ThemePreference },
  { label: '深色', value: 'dark' as ThemePreference }
];

const preference = computed(() => themeStore.preference);
const activeIndex = computed(() => themeOptions.findIndex((option) => option.value === preference.value));
const indicatorStyle = computed(() => ({
  transform: `translateX(${Math.max(activeIndex.value, 0) * 100}%)`
}));

const selectTheme = (mode: ThemePreference) => {
  themeStore.setPreference(mode);
};
</script>

<style scoped>
.theme-toggle {
  position: relative;
  display: inline-flex;
  align-items: center;
  justify-content: space-between;
  gap: 0;
  padding: 0.18rem;
  border-radius: var(--radius-full);
  border: var(--border-1);
  background: linear-gradient(120deg, color-mix(in srgb, var(--surface-subtle) 85%, transparent), var(--surface-default));
  box-shadow:
    inset 0 0 0 1px color-mix(in srgb, var(--color-border) 45%, transparent),
    0 6px 16px rgba(15, 23, 42, 0.12);
  overflow: hidden;
  min-width: 220px;
  transition:
    border-color 160ms ease,
    background-color 200ms ease,
    box-shadow 220ms ease;
}

.theme-toggle:focus-within {
  border-color: var(--color-ring);
  box-shadow:
    inset 0 0 0 1px color-mix(in srgb, var(--color-ring) 50%, transparent),
    0 0 0 3px color-mix(in srgb, var(--color-ring) 35%, transparent);
}

.theme-toggle__indicator {
  position: absolute;
  inset: 3px;
  width: calc((100% - 6px) / 3);
  height: calc(100% - 6px);
  border-radius: var(--radius-full);
  background: radial-gradient(circle at 20% 20%, color-mix(in srgb, var(--color-accent) 20%, transparent), transparent),
    var(--surface-default);
  box-shadow:
    inset 0 0 0 1px color-mix(in srgb, var(--color-border) 55%, transparent),
    var(--shadow-sm);
  transition: transform 260ms cubic-bezier(0.22, 1, 0.36, 1);
  will-change: transform;
}

.theme-toggle__option {
  position: relative;
  z-index: 1;
  flex: 1 1 0%;
  padding: 0.35rem 0.25rem;
  border: none;
  border-radius: var(--radius-full);
  background: transparent;
  color: var(--ink-muted);
  font-weight: 600;
  font-size: 0.85rem;
  line-height: 1.2;
  transition:
    color 160ms ease,
    transform 160ms ease;
}

.theme-toggle__option:focus-visible {
  outline: none;
  color: var(--ink-primary);
}

.theme-toggle__option--active {
  color: var(--ink-primary);
  transform: translateY(-1px);
}

.theme-toggle__option--active::after {
  content: '';
  position: absolute;
  inset-inline: 25%;
  bottom: 3px;
  height: 2px;
  border-radius: var(--radius-full);
  background: color-mix(in srgb, var(--color-accent) 70%, transparent);
  opacity: 0.65;
}

.theme-toggle__option:hover {
  color: var(--ink-primary);
}
</style>
