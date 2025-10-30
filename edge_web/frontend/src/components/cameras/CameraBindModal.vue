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
      <label class="grid gap-2">
        <span class="text-sm font-medium text-ink">靈敏度</span>
        <div class="flex items-center gap-3">
          <input
            v-model.number="form.fall_sensitivity"
            type="range"
            min="0"
            max="100"
            step="1"
            class="h-1.5 w-full cursor-pointer rounded-full bg-surface/60 outline-none"
          />
          <span class="w-10 text-right text-sm font-medium text-ink">{{ form.fall_sensitivity }}</span>
        </div>
      </label>
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

/**
  * 組件用途：引導使用者填寫攝影機綁定資料，維持原流程的確認步驟。
  * 輸入參數：open 控制顯示、camera 提供預設資訊、loading 顯示送出狀態。
  * 與其他模組關聯：CamerasView 會透過 submit 事件呼叫 cameraService。
  */

const props = defineProps<{
  open: boolean;
  camera: Camera | null;
  loading?: boolean;
}>();

const emit = defineEmits<{
  (e: 'close'): void;
  (
    e: 'submit',
    payload: { custom_name: string; ipc_account?: string; ipc_password?: string; fall_sensitivity: number }
  ): void;
}>();

const clampSensitivity = (value: unknown): number => {
  const numeric = typeof value === 'number' ? value : Number(value);
  if (Number.isFinite(numeric)) {
    return Math.min(100, Math.max(0, Math.round(numeric)));
  }
  return 70;
};

// 表單狀態：綁定前預填攝影機名稱並保留可修改欄位
const form = reactive({
  custom_name: '',
  ipc_account: '',
  ipc_password: '',
  fall_sensitivity: 70
});

// 監看選擇攝影機變化，預先填入名稱並清除帳密
watch(
  () => props.camera,
  (next) => {
    if (!next) return;
    form.custom_name = next.name ?? 'IPC';
    form.ipc_account = '';
    form.ipc_password = '';
    form.fall_sensitivity = clampSensitivity(next.fall_sensitivity ?? 70);
  },
  { immediate: true }
);

watch(
  () => props.open,
  (open) => {
    if (!open) return;
    form.fall_sensitivity = clampSensitivity(props.camera?.fall_sensitivity ?? 70);
  }
);

// 送出表單並透過事件將資料交還外層
const submit = () => {
  emit('submit', { ...form, fall_sensitivity: clampSensitivity(form.fall_sensitivity) });
};
</script>
