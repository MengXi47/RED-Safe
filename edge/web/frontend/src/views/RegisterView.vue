<template>
  <AuthLayout title="首次使用：設定密碼" subtitle="建立帳號以管理 RED Safe 儀表板">
    <template #logo>
      <div
        class="flex h-16 w-16 items-center justify-center rounded-2xl border border-border bg-surface-subtle text-2xl font-bold text-brand-600"
        aria-hidden="true"
      >
        RS
      </div>
    </template> 

    <div
      v-if="errorMessage"
      class="w-full rounded-2xl border border-danger/40 bg-danger/10 px-4 py-3 text-sm font-medium text-danger"
      role="alert"
    >
      {{ errorMessage }}
    </div>
    <form class="flex flex-col gap-6" @submit.prevent="handleSubmit" novalidate>
      <BaseInput
        id="register-email"
        v-model="email"
        type="email"
        label="Email"
        placeholder="you@example.com"
        autocomplete="email"
        :error="emailError"
        :invalid="Boolean(emailError)"
        required
        @blur="touched.email = true"
      />

      <PasswordInputField
        id="register-password"
        v-model="password"
        label="密碼"
        placeholder="請輸入密碼"
        autocomplete="new-password"
        :error="passwordError"
        :invalid="Boolean(passwordError)"
        required
        toggle-aria-show="顯示密碼"
        toggle-aria-hide="隱藏密碼"
        @blur="touched.password = true"
      />

      <PasswordInputField
        id="register-password-confirm"
        v-model="confirmPassword"
        label="確認密碼"
        placeholder="請再次輸入密碼"
        autocomplete="new-password"
        :error="confirmError"
        :invalid="Boolean(confirmError)"
        required
        toggle-aria-show="顯示確認密碼"
        toggle-aria-hide="隱藏確認密碼"
        @blur="touched.confirm = true"
      />

      <BaseButton type="submit" variant="primary" shape="rounded" full-width :loading="isSubmitting">
        建立帳號
      </BaseButton>
    </form>

    <template #footer>
      <p class="text-sm">
        已經有帳號？
        <RouterLink class="font-semibold text-brand-600 hover:text-brand-700" :to="{ name: 'Login' }">
          去登入
        </RouterLink>
      </p>
    </template>
  </AuthLayout>
</template>

<script setup lang="ts">
import { computed, reactive, ref, watch } from 'vue';
import { RouterLink, useRouter } from 'vue-router';
import AuthLayout from '@/layouts/AuthLayout.vue';
import BaseButton from '@/components/ui/BaseButton.vue';
import BaseInput from '@/components/ui/BaseInput.vue';
import PasswordInputField from '@/components/auth/PasswordInputField.vue';
import { useAuthThemeMeta } from '@/lib/useAuthThemeMeta';

const router = useRouter();

const email = ref('');
const password = ref('');
const confirmPassword = ref('');

const isSubmitting = ref(false);
const errorMessage = ref('');

const touched = reactive({
  email: false,
  password: false,
  confirm: false
});

useAuthThemeMeta();

// Email 驗證：檢查格式合法性與必填條件
const emailError = computed(() => {
  if (!touched.email && !email.value) return '';
  if (!email.value.trim()) return '請輸入 Email';
  const emailPattern = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
  if (!emailPattern.test(email.value)) return '請輸入有效的 Email';
  return '';
});

// 密碼驗證：套用長度與字元限制
const passwordError = computed(() => {
  if (!touched.password && !password.value) return '';
  if (!password.value.trim()) return '請輸入密碼';
  if (password.value.length < 6) return '密碼需至少 6 碼';
  if (!/^[A-Za-z0-9]+$/.test(password.value)) return '僅能使用英文或數字';
  return '';
});

// 確認密碼驗證：再次檢查一致性
const confirmError = computed(() => {
  if (!touched.confirm && !confirmPassword.value) return '';
  if (!confirmPassword.value.trim()) return '請再次輸入密碼';
  if (confirmPassword.value.length < 6) return '密碼需至少 6 碼';
  if (!/^[A-Za-z0-9]+$/.test(confirmPassword.value)) return '僅能使用英文或數字';
  if (confirmPassword.value !== password.value) return '兩次輸入不一致';
  return '';
});

// 重置伺服器錯誤訊息，避免使用者修正輸入後仍顯示錯誤
watch([email, password, confirmPassword], () => {
  errorMessage.value = '';
});

const hasErrors = computed(
  () => Boolean(emailError.value || passwordError.value || confirmError.value)
);

// 表單送出：先行驗證再模擬 API 呼叫與導頁
const handleSubmit = async () => {
  touched.email = true;
  touched.password = true;
  touched.confirm = true;

  if (hasErrors.value) {
    return;
  }

  isSubmitting.value = true;
  errorMessage.value = '';

  try {
    await new Promise((resolve) => setTimeout(resolve, 800));
    // TODO：待與後端註冊 API 串接後送出實際請求
    router.push({ name: 'Login' });
  } catch {
    errorMessage.value = '註冊失敗，請稍後再試。';
  } finally {
    isSubmitting.value = false;
  }
};
</script>
