<template>
  <BaseModal :open="open" title="綁定攝影機" description="輸入必要資訊後完成綁定" @close="emit('close')">
    <div class="grid gap-4">
      <div class="grid grid-cols-2 gap-3 rounded-xl bg-surface-subtle p-3">
        <div>
          <p class="text-xs text-ink-muted">IP 位址</p>
          <p class="font-medium text-ink">{{ camera?.ip ?? '--' }}</p>
        </div>
        <div>
          <p class="text-xs text-ink-muted">MAC 位址</p>
          <p class="font-medium text-ink">{{ camera?.mac ?? '--' }}</p>
        </div>
      </div>
      <BaseInput v-model="form.custom_name" label="顯示名稱" placeholder="例如：大門攝影機" />
      <BaseInput v-model="form.ipc_account" label="登入帳號" placeholder="選填" />
      <BaseInput v-model="form.ipc_password" label="登入密碼" placeholder="選填" type="password" />
    </div>
    <template #footer>
      <BaseButton variant="ghost" @click="emit('close')">取消</BaseButton>
      <BaseButton :loading="loading" @click="submit">確定綁定</BaseButton>
    </template>
  </BaseModal>
</template>

<script setup lang="ts">
import { reactive, watch } from 'vue';
import BaseModal from '@/components/ui/BaseModal.vue';
import BaseInput from '@/components/ui/BaseInput.vue';
import BaseButton from '@/components/ui/BaseButton.vue';
import type { Camera } from '@/types/camera';

const props = defineProps<{
  open: boolean;
  camera: Camera | null;
  loading?: boolean;
}>();

const emit = defineEmits<{
  (e: 'close'): void;
  (e: 'submit', payload: { custom_name: string; ipc_account?: string; ipc_password?: string }): void;
}>();

const form = reactive({
  custom_name: '',
  ipc_account: '',
  ipc_password: ''
});

watch(
  () => props.camera,
  (next) => {
    if (!next) return;
    form.custom_name = next.name ?? 'IPC';
    form.ipc_account = '';
    form.ipc_password = '';
  },
  { immediate: true }
);

const submit = () => {
  emit('submit', { ...form });
};
</script>
