<template>
  <BaseInput
    :id="id"
    :model-value="modelValue"
    :label="label"
    :placeholder="placeholder"
    :type="isVisible ? 'text' : 'password'"
    :autocomplete="autocomplete"
    :error="error"
    :help="help"
    :invalid="invalid"
    :required="required"
    :disabled="disabled"
    :maxlength="maxlength"
    @update:modelValue="onUpdate"
    @blur="$emit('blur')"
  >
    <template #append>
      <button
        type="button"
        class="rounded-xl px-3 py-2 text-sm font-semibold text-brand-600 transition hover:text-brand-700 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-brand-100"
        :aria-pressed="isVisible ? 'true' : 'false'"
        :aria-label="ariaLabel"
        @click="toggleVisibility"
      >
        <span aria-hidden="true">{{ buttonLabel }}</span>
      </button>
    </template>
  </BaseInput>
</template>

<script setup lang="ts">
import { computed, ref } from 'vue';
import BaseInput from '@/components/ui/BaseInput.vue';

/**
  * 組件用途：提供一致的密碼輸入欄位與顯示/隱藏按鈕樣式。
  * 輸入參數：透過 props 控制標籤、占位字、錯誤狀態與自訂 ARIA 文案。
  * 與其他模組關聯：LoginView、RegisterView 共用以維持主視覺一致。
  */

const props = withDefaults(
  defineProps<{
    modelValue: string;
    label: string;
    placeholder?: string;
    id?: string;
    autocomplete?: string;
    error?: string;
    help?: string;
    invalid?: boolean;
    required?: boolean;
    disabled?: boolean;
    maxlength?: number;
    toggleLabelShow?: string;
    toggleLabelHide?: string;
    toggleAriaShow?: string;
    toggleAriaHide?: string;
  }>(),
  {
    placeholder: '',
    id: undefined,
    autocomplete: 'off',
    error: '',
    help: '',
    invalid: false,
    required: false,
    disabled: false,
    maxlength: undefined,
    toggleLabelShow: '顯示',
    toggleLabelHide: '隱藏',
    toggleAriaShow: '顯示密碼',
    toggleAriaHide: '隱藏密碼'
  }
);

const emit = defineEmits<{
  (e: 'update:modelValue', value: string): void;
  (e: 'blur'): void;
}>();

const isVisible = ref(false);

const buttonLabel = computed(() =>
  isVisible.value ? props.toggleLabelHide : props.toggleLabelShow
);

const ariaLabel = computed(() =>
  isVisible.value ? props.toggleAriaHide : props.toggleAriaShow
);

const toggleVisibility = () => {
  isVisible.value = !isVisible.value;
};

const onUpdate = (value: string) => {
  emit('update:modelValue', value);
};
</script>
