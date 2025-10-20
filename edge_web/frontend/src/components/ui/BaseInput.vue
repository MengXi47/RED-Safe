<template>
  <label
    class="flex w-full flex-col gap-2 text-sm text-ink"
    :class="{ 'cursor-not-allowed opacity-70': disabled }"
  >
    <div v-if="label" class="flex items-center justify-between">
      <span class="text-sm font-semibold text-ink">
        {{ label }}
        <span v-if="required" aria-hidden="true" class="ml-1 text-danger">*</span>
      </span>
      <slot name="labelSuffix" />
    </div>

    <div
      class="group relative flex min-h-[3rem] items-center gap-2 rounded-2xl border border-border bg-surface-default px-4 py-2 shadow-elev-sm transition focus-within:border-brand-500 focus-within:outline-none focus-within:ring-4 focus-within:ring-brand-100/70"
      :class="{
        'border-danger focus-within:border-danger focus-within:ring-danger/30': hasError,
        'bg-surface-subtle text-ink-muted': disabled
      }"
    >
      <slot name="icon" />
      <input
        v-bind="inputAttrs"
        :id="inputId"
        :value="modelValue"
        :placeholder="placeholder"
        :type="type"
        :disabled="disabled"
        :autocomplete="autocomplete"
        :required="required"
        :maxlength="maxlength"
        :aria-invalid="hasError ? 'true' : undefined"
        :aria-describedby="describedBy"
        class="flex-1 border-none bg-transparent text-base text-ink placeholder:text-ink-muted placeholder:opacity-70 focus:outline-none"
        @input="onInput"
        @blur="$emit('blur')"
      />
      <slot name="append" />
    </div>

    <p v-if="displayHelp" :id="helpId" class="text-xs text-ink-muted">
      {{ displayHelp }}
    </p>
    <p v-if="error" :id="errorId" class="text-xs font-medium text-danger">
      {{ error }}
    </p>
  </label>
</template>

<script setup lang="ts">
import { computed, useAttrs, useId } from 'vue';

/**
  * 組件用途：建立表單輸入欄位基礎樣式與驗證狀態呈現，支援圖示與附加區塊。
  * 輸入參數：modelValue、label、error、help、type、disabled、required 等。
  * 與其他模組關聯：登入/註冊與各模態表單共同使用，確保輸入體驗一致。
  */

defineOptions({
  inheritAttrs: false
});

const props = withDefaults(
  defineProps<{
    modelValue?: string;
    label?: string;
    placeholder?: string;
    error?: string;
    help?: string;
    helpText?: string;
    type?: string;
    disabled?: boolean;
    autocomplete?: string;
    required?: boolean;
    maxlength?: number;
    invalid?: boolean;
  }>(),
  {
    modelValue: '',
    label: '',
    placeholder: '',
    error: '',
    help: '',
    helpText: '',
    type: 'text',
    disabled: false,
    autocomplete: 'off',
    required: false,
    maxlength: undefined,
    invalid: false
  }
);

const emit = defineEmits<{
  (event: 'update:modelValue', value: string): void;
  (event: 'blur'): void;
}>();

const attrs = useAttrs();
const generatedId = useId();
const inputId = computed(() => (attrs.id as string) || generatedId);
const helpId = computed(() => `${inputId.value}-help`);
const errorId = computed(() => `${inputId.value}-error`);

const displayHelp = computed(() => props.help || props.helpText);

const hasError = computed(() => Boolean(props.error) || props.invalid);

const describedBy = computed(() => {
  if (hasError.value && props.error) return errorId.value;
  if (displayHelp.value) return helpId.value;
  return undefined;
});

const inputAttrs = computed(() => {
  const result = { ...attrs } as Record<string, unknown>;
  delete result.id;
  delete result.class;
  delete result.value;
  return result;
});

const onInput = (event: Event) => {
  emit('update:modelValue', (event.target as HTMLInputElement).value);
};
</script>
