<template>
  <div class="space-y-8">
    <header class="page-hero">
      <h2 class="text-2xl font-semibold text-ink">裝置資訊</h2>
      <p class="mt-1 text-sm text-ink-muted">快速檢視裝置序號、版本與預設登入資訊。</p>
    </header>
    <BaseCard>
      <dl class="grid gap-4 md:grid-cols-2">
        <div class="rounded-xl bg-surface-subtle p-4">
          <dt class="text-xs text-ink-muted">序號</dt>
          <dd class="mt-1 text-base font-semibold text-ink">{{ info.serial }}</dd>
        </div>
        <div class="rounded-xl bg-surface-subtle p-4">
          <dt class="text-xs text-ink-muted">版本</dt>
          <dd class="mt-1 text-base font-semibold text-ink">{{ info.version }}</dd>
        </div>
        <div class="rounded-xl bg-surface-subtle p-4">
          <dt class="text-xs text-ink-muted">裝置密碼</dt>
          <dd class="mt-1 flex items-center gap-2">
            <span class="text-base font-semibold text-ink">{{ displayPassword }}</span>
            <BaseButton size="sm" variant="ghost" @click="togglePassword">
              {{ passwordVisible ? '隱藏' : '顯示' }}
            </BaseButton>
          </dd>
        </div>
        <div class="rounded-xl bg-surface-subtle p-4">
          <dt class="text-xs text-ink-muted">連線狀態</dt>
          <dd class="mt-1 text-base font-semibold" :class="info.status === 1 ? 'text-success' : 'text-danger'">
            {{ info.status === 1 ? '已連線' : '未連線' }}
          </dd>
        </div>
      </dl>
      <div v-if="qrcode" class="mt-6 flex flex-col items-center gap-3">
        <img :src="qrcode" alt="device qr" class="h-64 w-64 rounded-3xl border border-border shadow-elev-sm" />
        <BaseButton variant="ghost" @click="downloadQr">下載 QRCode</BaseButton>
      </div>
    </BaseCard>
  </div>
</template>

<script setup lang="ts">
import { computed, ref } from 'vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import BaseButton from '@/components/ui/BaseButton.vue';

interface DeviceInfoState {
  serial: string;
  version: string;
  status: number;
  masked_password: string;
  password: string;
}

const initial = (window as Window & { __EDGE_INITIAL_STATE__?: any }).__EDGE_INITIAL_STATE__?.device ?? {};

const info = computed<DeviceInfoState>(() => ({
  serial: initial.serial ?? 'RED-UNKNOWN',
  version: initial.version ?? 'v1.0.0',
  status: initial.status ?? 0,
  masked_password: initial.masked_password ?? '******',
  password: initial.password ?? ''
}));

const qrcode = initial.qrcode ?? '';
const passwordVisible = ref(false);

const displayPassword = computed(() => (passwordVisible.value ? info.value.password : info.value.masked_password));

const togglePassword = () => {
  passwordVisible.value = !passwordVisible.value;
};

const downloadQr = () => {
  const link = document.createElement('a');
  link.href = qrcode;
  link.download = `device-${info.value.serial}.png`;
  link.click();
};
</script>
