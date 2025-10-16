<template>
  <label class="flex flex-col gap-1 text-sm text-ink">
    <span v-if="label" class="font-medium">{{ label }}</span>
    <div class="relative">
      <input
        v-bind="attrs"
        :value="modelValue"
        :placeholder="placeholder"
        :disabled="disabled"
        @input="onInput"
        @blur="$emit('blur')"
        class="w-full rounded-lg border border-slate-200 bg-white px-3 py-2 text-base text-ink shadow-sm transition focus:border-brand-500 focus:outline-none focus:ring-2 focus:ring-brand-100 disabled:bg-slate-100"
        :aria-describedby="helpText ? helpId : undefined"
        :aria-invalid="invalid || undefined"
      />
      <slot name="icon" />
    </div>
    <p v-if="helpText" :id="helpId" class="text-xs text-ink-muted">{{ helpText }}</p>
    <p v-if="error" class="text-xs text-danger">{{ error }}</p>
  </label>
</template>

<script setup lang="ts">
import { computed, useAttrs } from 'vue';

defineOptions({
  inheritAttrs: false
});

const props = withDefaults(
  defineProps<{
    modelValue?: string | number;
    label?: string;
    placeholder?: string;
    helpText?: string;
    error?: string;
    disabled?: boolean;
    invalid?: boolean;
  }>(),
  {
    modelValue: '',
    placeholder: '',
    helpText: '',
    error: '',
    disabled: false,
    invalid: false
  }
);

const emit = defineEmits(['update:modelValue', 'blur']);

const attrs = useAttrs();
const helpId = computed(() => `${attrs.id ?? 'input'}-help`);

const onInput = (event: Event) => {
  const target = event.target as HTMLInputElement;
  emit('update:modelValue', target.value);
};
</script>
