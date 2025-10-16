<template>
  <div class="mx-auto max-w-lg space-y-8">
    <header class="page-hero">
      <h2 class="text-2xl font-semibold text-ink">修改登入密碼</h2>
      <p class="mt-1 text-sm text-ink-muted">更新裝置密碼後需重新登入確認。</p>
    </header>
    <BaseCard>
      <form class="space-y-4" @submit.prevent="submit">
        <BaseInput v-model="form.old_password" label="舊密碼" type="password" :error="errors.old_password" />
        <BaseInput
          v-model="form.new_password1"
          label="新密碼"
          type="password"
          help-text="至少 6 碼，僅限英數"
          :error="errors.new_password1"
        />
        <BaseInput
          v-model="form.new_password2"
          label="再次輸入新密碼"
          type="password"
          :error="errors.new_password2"
        />
        <div class="flex justify-end gap-3">
          <BaseButton type="submit" :loading="submitting">確認修改</BaseButton>
        </div>
      </form>
    </BaseCard>
  </div>
</template>

<script setup lang="ts">
import { reactive, ref } from 'vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import BaseInput from '@/components/ui/BaseInput.vue';
import BaseButton from '@/components/ui/BaseButton.vue';
import { useUiStore } from '@/store/ui';

const initial = (window as Window & { __EDGE_INITIAL_STATE__?: any }).__EDGE_INITIAL_STATE__ ?? {};

const form = reactive({
  old_password: '',
  new_password1: '',
  new_password2: ''
});
const errors = reactive<{ [key: string]: string }>({ ...(initial.formErrors ?? {}) });
const submitting = ref(false);
const uiStore = useUiStore();

const submit = async () => {
  submitting.value = true;
  Object.keys(errors).forEach((key) => delete errors[key]);
  try {
    const formData = new FormData();
    formData.append('old_password', form.old_password);
    formData.append('new_password1', form.new_password1);
    formData.append('new_password2', form.new_password2);
    formData.append('csrfmiddlewaretoken', getCsrfToken());

    const response = await fetch('/device/device_change-password/', {
      method: 'POST',
      body: formData
    });
    if (response.redirected) {
      window.location.href = response.url;
      return;
    }
    const html = await response.text();
    parseErrors(html);
    if (Object.keys(errors).length === 0) {
      uiStore.pushToast('密碼更新完成，請重新登入', 'success');
    }
  } catch (error) {
    console.error(error);
    uiStore.pushToast('更新失敗，請稍後再試', 'danger');
  } finally {
    submitting.value = false;
  }
};

const getCsrfToken = () => {
  const match = document.cookie.match(/csrftoken=([^;]+)/);
  return match ? match[1] : '';
};

const parseErrors = (html: string) => {
  const parser = new DOMParser();
  const doc = parser.parseFromString(html, 'text/html');
  const script = doc.getElementById('edge-initial-state');
  if (!script?.textContent) return;
  try {
    const payload = JSON.parse(script.textContent) as { formErrors?: Record<string, string> };
    if (payload.formErrors) {
      Object.assign(errors, payload.formErrors);
    }
  } catch (error) {
    console.warn('無法解析錯誤訊息', error);
  }
};
</script>
