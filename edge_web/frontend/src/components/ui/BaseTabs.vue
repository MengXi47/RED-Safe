<template>
  <div>
    <div class="flex gap-2 border-b border-slate-200">
      <button
        v-for="tab in tabs"
        :key="tab.value"
        type="button"
        role="tab"
        :aria-selected="innerValue === tab.value"
        :class="tabButtonClass(tab.value)"
        @click="select(tab.value)"
      >
        {{ tab.label }}
      </button>
    </div>
    <div class="pt-4">
      <slot :active="innerValue" />
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue';

interface Tab {
  value: string;
  label: string;
}

const props = defineProps<{
  modelValue: string;
  tabs: Tab[];
}>();

const emit = defineEmits<{
  (e: 'update:modelValue', value: string): void;
}>();

const innerValue = computed(() => props.modelValue);

const select = (value: string) => {
  emit('update:modelValue', value);
};

const tabButtonClass = (value: string) => {
  const base =
    'relative rounded-t-lg px-4 py-2 text-sm font-medium transition focus-visible:outline focus-visible:outline-2 focus-visible:outline-brand-200';
  const active =
    'bg-surface-default text-brand-600 shadow-soft after:absolute after:bottom-[-1px] after:left-0 after:right-0 after:h-0.5 after:bg-brand-500';
  const inactive = 'text-ink-muted hover:text-brand-600';
  return [base, innerValue.value === value ? active : inactive].join(' ');
};
</script>
