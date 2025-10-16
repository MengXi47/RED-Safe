<template>
  <label class="flex cursor-pointer items-center gap-3 text-sm text-ink">
    <input
      type="checkbox"
      class="peer sr-only"
      :checked="modelValue"
      :disabled="disabled"
      @change="onChange"
    />
    <span
      class="flex h-5 w-5 items-center justify-center rounded border border-slate-300 bg-white transition peer-checked:border-brand-500 peer-checked:bg-brand-500 peer-focus-visible:outline peer-focus-visible:outline-2 peer-focus-visible:outline-brand-200 peer-disabled:opacity-60"
      aria-hidden="true"
    >
      <svg v-if="modelValue" class="h-3 w-3 text-white" viewBox="0 0 24 24" fill="none">
        <path d="M5 13l4 4L19 7" stroke="currentColor" stroke-width="3" stroke-linecap="round" />
      </svg>
    </span>
    <span>
      <span class="font-medium">{{ label }}</span>
      <span v-if="description" class="block text-xs text-ink-muted">{{ description }}</span>
    </span>
  </label>
</template>

<script setup lang="ts">
const props = withDefaults(
  defineProps<{
    modelValue?: boolean;
    label: string;
    description?: string;
    disabled?: boolean;
  }>(),
  {
    modelValue: false,
    description: '',
    disabled: false
  }
);

const emit = defineEmits<{
  (e: 'update:modelValue', value: boolean): void;
}>();

const onChange = (event: Event) => {
  const target = event.target as HTMLInputElement;
  emit('update:modelValue', target.checked);
};
</script>
