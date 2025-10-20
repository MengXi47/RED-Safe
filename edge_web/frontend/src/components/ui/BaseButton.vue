<template>
  <button
    :type="type"
    :class="buttonClasses"
    :disabled="disabled || loading"
    :aria-label="ariaLabel"
    :aria-busy="loading ? 'true' : undefined"
    v-bind="forwardedAttrs"
  >
    <span
      v-if="loading"
      class="mr-2 inline-flex h-4 w-4 items-center justify-center"
      aria-hidden="true"
    >
      <BaseLoadingSpinner :size="size === 'sm' ? 'xs' : 'sm'" />
    </span>
    <span v-else-if="icon" class="mr-2 flex items-center" aria-hidden="true">
      <component :is="icon" class="h-4 w-4" />
    </span>
    <span class="flex items-center gap-2">
      <slot />
    </span>
  </button>
</template>

<script setup lang="ts">
import { computed, useAttrs } from 'vue';
import type { Component } from 'vue';
import BaseLoadingSpinner from './BaseLoadingSpinner.vue';

/**
  * 組件用途：提供全站一致的按鈕樣式與狀態，支援載入、寬度與圖示配置。
  * 輸入參數：type、variant、size、icon、fullWidth、shape、ariaLabel 等。
  * 與其他模組關聯：廣泛用於各頁面操作，並搭配 BaseLoadingSpinner 呈現載入狀態。
  */

type Variant = 'primary' | 'secondary' | 'ghost' | 'danger' | 'text';
type Size = 'sm' | 'md' | 'lg';
type Shape = 'pill' | 'rounded';

const props = withDefaults(
  defineProps<{
    type?: 'button' | 'submit' | 'reset';
    variant?: Variant;
    size?: Size;
    disabled?: boolean;
    loading?: boolean;
    icon?: Component | null;
    fullWidth?: boolean;
    ariaLabel?: string;
    shape?: Shape;
  }>(),
  {
    type: 'button',
    variant: 'primary',
    size: 'md',
    disabled: false,
    loading: false,
    icon: null,
    fullWidth: false,
    ariaLabel: undefined,
    shape: 'pill'
  }
);

const attrs = useAttrs();

const forwardedAttrs = computed(() => {
  const rest = { ...attrs } as Record<string, unknown>;
  delete rest.class;
  return rest;
});

const buttonClasses = computed(() => {
  const base =
    'inline-flex items-center justify-center font-semibold transition-colors focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 disabled:cursor-not-allowed disabled:opacity-60';

  const shapeClass: Record<Shape, string> = {
    pill: 'rounded-full',
    rounded: 'rounded-2xl'
  };

  const sizeClass: Record<Size, string> = {
    sm: 'h-8 px-3 text-sm gap-1',
    md: 'h-10 px-4 text-sm gap-2',
    lg: 'h-12 px-6 text-base gap-2'
  };

  const variantClass: Record<Variant, string> = {
    primary:
      'bg-brand-500 text-white shadow-elev-sm hover:bg-brand-600 focus-visible:outline-brand-500 active:bg-brand-700',
    secondary:
      'bg-surface-default text-ink border border-border shadow-elev-sm hover:border-brand-500 hover:text-brand-600 focus-visible:outline-brand-500 active:bg-brand-50/60 disabled:bg-surface-subtle disabled:text-ink-muted',
    ghost:
      'bg-transparent text-ink hover:bg-brand-50/70 focus-visible:outline-brand-500 active:bg-brand-100/70',
    danger:
      'bg-danger text-white shadow-elev-sm hover:bg-danger/90 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-danger/50 active:bg-danger/80',
    text:
      'bg-transparent text-brand-600 shadow-none hover:text-brand-700 hover:underline focus-visible:outline-brand-500'
  };

  const extraClass = attrs.class as string | undefined;

  return [
    base,
    shapeClass[props.shape],
    sizeClass[props.size],
    variantClass[props.variant],
    props.fullWidth ? 'w-full' : '',
    props.loading ? 'cursor-wait' : '',
    extraClass
  ]
    .filter(Boolean)
    .join(' ');
});
</script>
