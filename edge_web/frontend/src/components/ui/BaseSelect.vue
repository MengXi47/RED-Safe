<template>
  <label class="flex flex-col gap-1 text-sm text-ink">
    <span v-if="label" class="font-medium">{{ label }}</span>
    <div class="relative">
      <select
        v-bind="attrs"
        :value="modelValue"
        :disabled="disabled"
        @change="onChange"
        class="w-full appearance-none rounded-lg border border-border bg-surface-default px-3 py-2 text-base text-ink shadow-elev-sm transition focus:border-brand-500 focus:outline-none focus:ring-2 focus:ring-brand-100 disabled:bg-surface-subtle"
        :aria-describedby="helpText ? helpId : undefined"
      >
        <slot />
      </select>
      <span class="pointer-events-none absolute inset-y-0 right-3 flex items-center text-ink-muted">
        <svg class="h-4 w-4" viewBox="0 0 24 24" fill="none">
          <path d="M6 9l6 6 6-6" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" />
        </svg>
      </span>
    </div>
    <p v-if="helpText" :id="helpId" class="text-xs text-ink-muted">{{ helpText }}</p>
    <p v-if="error" class="text-xs text-danger">{{ error }}</p>
  </label>
</template>

<script setup lang="ts">
import { computed, useAttrs } from 'vue';

defineOptions({ inheritAttrs: false });

const props = withDefaults(
  defineProps<{
    modelValue?: string | number;
    label?: string;
    helpText?: string;
    error?: string;
    disabled?: boolean;
  }>(),
  {
    modelValue: '',
    label: '',
    helpText: '',
    error: '',
    disabled: false
  }
);

const emit = defineEmits(['update:modelValue']);
const attrs = useAttrs();

const helpId = computed(() => `${attrs.id ?? 'select'}-help`);

const onChange = (event: Event) => {
  const target = event.target as HTMLSelectElement;
  emit('update:modelValue', target.value);
};
</script>
