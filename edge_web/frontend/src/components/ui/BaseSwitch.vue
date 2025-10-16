<template>
  <label class="flex items-center gap-3 text-sm text-ink">
    <button
      type="button"
      role="switch"
      :aria-checked="modelValue"
      :class="switchClass"
      :disabled="disabled"
      @click="toggle"
    >
      <span :class="thumbClass" />
    </button>
    <div class="flex flex-col">
      <span class="font-medium">{{ label }}</span>
      <span v-if="description" class="text-xs text-ink-muted">{{ description }}</span>
    </div>
  </label>
</template>

<script setup lang="ts">
import { computed } from 'vue';

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

const switchClass = computed(() => [
  'relative inline-flex h-6 w-11 items-center rounded-full transition-colors focus-visible:outline focus-visible:outline-2 focus-visible:outline-brand-200',
  props.modelValue ? 'bg-brand-500' : 'bg-slate-300',
  props.disabled ? 'cursor-not-allowed opacity-60' : 'cursor-pointer'
]);

const thumbClass = computed(() => [
  'inline-block h-5 w-5 transform rounded-full bg-white transition-transform shadow',
  props.modelValue ? 'translate-x-5' : 'translate-x-1'
]);

const toggle = () => {
  if (props.disabled) return;
  emit('update:modelValue', !props.modelValue);
};
</script>
