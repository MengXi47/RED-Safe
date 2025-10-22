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

      <BaseInput
        id="register-password"
        v-model="password"
        :type="showPassword ? 'text' : 'password'"
        label="密碼"
        placeholder="至少 6 碼，限英數字"
        autocomplete="new-password"
        :error="passwordError"
        :invalid="Boolean(passwordError)"
        help="至少 6 碼，僅限英文或數字"
        required
        @blur="touched.password = true"
      >
        <template #append>
          <button
            type="button"
            class="rounded-xl px-3 py-2 text-sm font-semibold text-brand-600 transition hover:text-brand-700 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-brand-100"
            :aria-pressed="showPassword ? 'true' : 'false'"
            :aria-label="showPassword ? '隱藏密碼' : '顯示密碼'"
            @click="toggleVisibility('password')"
          >
            <span aria-hidden="true">{{ showPassword ? '隱藏' : '顯示' }}</span>
          </button>
        </template>
      </BaseInput>

      <BaseInput
        id="register-password-confirm"
        v-model="confirmPassword"
        :type="showConfirm ? 'text' : 'password'"
        label="確認密碼"
        placeholder="再次輸入確認"
        autocomplete="new-password"
        :error="confirmError"
        :invalid="Boolean(confirmError)"
        required
        @blur="touched.confirm = true"
      >
        <template #append>
          <button
            type="button"
            class="rounded-xl px-3 py-2 text-sm font-semibold text-brand-600 transition hover:text-brand-700 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-brand-100"
            :aria-pressed="showConfirm ? 'true' : 'false'"
            :aria-label="showConfirm ? '隱藏確認密碼' : '顯示確認密碼'"
            @click="toggleVisibility('confirm')"
          >
            <span aria-hidden="true">{{ showConfirm ? '隱藏' : '顯示' }}</span>
          </button>
        </template>
      </BaseInput>

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
import { useAuthThemeMeta } from '@/lib/useAuthThemeMeta';

/**
  * 組件用途：提供首次使用者設定密碼的註冊表單，並維持既有動線。
  * 輸入參數：無外部 props，內部整合 BaseInput、BaseButton 與 Vue Router 導頁。
  * 與其他模組關聯：運用 AuthLayout 版型與 useAuthThemeMeta 調整主題色。
  */

const router = useRouter();

const email = ref('');
const password = ref('');
const confirmPassword = ref('');

const showPassword = ref(false);
const showConfirm = ref(false);
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

const toggleVisibility = (field: 'password' | 'confirm') => {
  if (field === 'password') {
    showPassword.value = !showPassword.value;
  } else {
    showConfirm.value = !showConfirm.value;
  }
};

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
