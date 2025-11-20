<template>
  <AuthLayout title="輸入密碼以登入系統" subtitle="請輸入管理密碼以繼續">
    <template #logo>
      <div
        class="flex h-16 w-16 items-center justify-center rounded-2xl border border-border bg-surface-subtle text-2xl font-bold text-brand-600"
        aria-hidden="true"
      >
        RS
      </div>
    </template>

    <div v-if="errorMessage" class="w-full rounded-2xl border border-danger/40 bg-danger/10 px-4 py-3 text-sm font-medium text-danger" role="alert">
      {{ errorMessage }}
    </div>

    <form class="flex flex-col gap-6" @submit.prevent="handleSubmit" novalidate>
      <PasswordInputField
        id="login-password"
        v-model="password"
        label="密碼"
        placeholder="請輸入密碼"
        autocomplete="current-password"
        :error="passwordError"
        :invalid="Boolean(passwordError)"
        required
        @blur="passwordTouched = true"
      />

      <BaseButton type="submit" variant="primary" shape="rounded" full-width :loading="isSubmitting">
        登入
      </BaseButton>
    </form>

    <template #footer>
      <p class="text-sm">
        還沒有帳號？
        <RouterLink class="font-semibold text-brand-600 hover:text-brand-700" :to="{ name: 'Register' }">
          去註冊
        </RouterLink>
      </p>
    </template>
  </AuthLayout>
</template>

<script setup lang="ts">
import { computed, ref } from 'vue';
import { RouterLink } from 'vue-router';
import AuthLayout from '@/layouts/AuthLayout.vue';
import BaseButton from '@/components/ui/BaseButton.vue';
import PasswordInputField from '@/components/auth/PasswordInputField.vue';
import { useAuthThemeMeta } from '@/lib/useAuthThemeMeta';

/**
  * 組件用途：顯示登入表單並驗證密碼輸入，維持原本的 UI 與流程。
  * 輸入參數：無外部 props，內部使用 PasswordInputField 與 BaseButton 管理互動。
  * 與其他模組關聯：使用 AuthLayout 呈現版型，透過 useAuthThemeMeta 設定登入頁主題色。
  */

const password = ref('');
const passwordTouched = ref(false);
const isSubmitting = ref(false);
const errorMessage = ref('');

useAuthThemeMeta();

// 密碼格式驗證：同步檢查空白、長度與字元限制
const passwordError = computed(() => {
  if (!passwordTouched.value && !password.value) return '';
  if (!password.value.trim()) return '請輸入密碼';
  if (password.value.length < 6) return '密碼需至少 6 碼';
  if (!/^[A-Za-z0-9]+$/.test(password.value)) return '僅能使用英文或數字';
  return '';
});

// 提交表單前檢核格式並模擬登入流程，保留原始行為
const handleSubmit = async () => {
  passwordTouched.value = true;
  if (passwordError.value) {
    return;
  }

  isSubmitting.value = true;
  errorMessage.value = '';

  try {
    await new Promise((resolve) => setTimeout(resolve, 600));
    // TODO：待與後端登入 API 串接後導向實際頁面
  } catch {
    errorMessage.value = '登入失敗，請稍後再試。';
  } finally {
    isSubmitting.value = false;
  }
};
</script>
