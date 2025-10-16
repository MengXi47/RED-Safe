<template>
  <button
    :type="type"
    :class="buttonClasses"
    :disabled="disabled || loading"
    v-bind="computedAttrs"
  >
    <span v-if="loading" class="mr-2 h-4 w-4">
      <BaseLoadingSpinner size="sm" />
    </span>
    <span v-if="icon && !loading" class="mr-2 flex items-center">
      <component :is="icon" class="h-4 w-4" />
    </span>
    <slot />
  </button>
</template>

<script setup lang="ts">
import { computed } from 'vue';
import BaseLoadingSpinner from './BaseLoadingSpinner.vue';

type Variant = 'primary' | 'secondary' | 'ghost' | 'danger';
type Size = 'sm' | 'md' | 'lg';

interface Props {
  type?: 'button' | 'submit' | 'reset';
  variant?: Variant;
  size?: Size;
  disabled?: boolean;
  loading?: boolean;
  icon?: any;
  ariaLabel?: string;
}

const props = withDefaults(defineProps<Props>(), {
  type: 'button',
  variant: 'primary',
  size: 'md',
  disabled: false,
  loading: false
});

const computedAttrs = computed(() => ({
  'aria-label': props.ariaLabel
}));

const buttonClasses = computed(() => {
  const base =
    'inline-flex items-center justify-center rounded-full font-medium transition-colors focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 disabled:cursor-not-allowed disabled:opacity-60';

  const sizeClass = {
    sm: 'h-8 px-3 text-sm gap-1',
    md: 'h-10 px-4 text-sm gap-2',
    lg: 'h-12 px-6 text-base gap-2'
  }[props.size];

  const variantClass = {
    primary:
      'bg-brand-500 text-white shadow-elev-sm hover:bg-brand-600 focus-visible:outline-brand-500 active:bg-brand-700',
    secondary:
      'bg-surface-default text-ink border border-border shadow-elev-sm hover:border-brand-500 hover:text-brand-600 focus-visible:outline-brand-500 active:bg-brand-50/60 disabled:bg-surface-subtle disabled:text-ink-muted',
    ghost:
      'bg-transparent text-ink hover:bg-brand-50/70 focus-visible:outline-brand-500 active:bg-brand-100/70',
    danger:
      'bg-danger text-white shadow-elev-sm hover:bg-danger/90 focus-visible:ring-2 focus-visible:ring-danger/50 focus-visible:outline-none active:bg-danger/80'
  }[props.variant];

  return [base, sizeClass, variantClass].join(' ');
});
</script>
