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
import { useInitialState } from '@/lib/useInitialState';
import type { DeviceInfo } from '@/types/device';

/**
  * 組件用途：呈現裝置序號、韌體版本與預設密碼資訊。
  * 輸入參數：無，透過伺服器注入資料建構初始狀態。
  * 與其他模組關聯：搭配 BaseCard 展現資訊並提供 QRCode 下載。
  */

const initialDevice = useInitialState<Partial<DeviceInfo>>(
  (state) => state.device ?? {},
  () => ({})
);

// 將初始資料整理為帶預設值的裝置資訊
const info = computed<DeviceInfo>(() => ({
  serial: initialDevice.serial ?? 'RED-UNKNOWN',
  version: initialDevice.version ?? 'v1.0.0',
  status: initialDevice.status ?? 0,
  masked_password: initialDevice.masked_password ?? '******',
  password: initialDevice.password ?? ''
}));

const qrcode = initialDevice.qrcode ?? '';
const passwordVisible = ref(false);

// 依據顯示狀態切換密碼或遮罩文字
const displayPassword = computed(() => (passwordVisible.value ? info.value.password : info.value.masked_password));

// 切換密碼顯示與否
const togglePassword = () => {
  passwordVisible.value = !passwordVisible.value;
};

// 建立隱藏下載連結，讓使用者保存 QRCode 圖檔
const downloadQr = () => {
  const link = document.createElement('a');
  link.href = qrcode;
  link.download = `device-${info.value.serial}.png`;
  link.click();
};
</script>
