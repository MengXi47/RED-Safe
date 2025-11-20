<template>
  <div class="space-y-8">
    <header class="page-hero">
      <h2 class="text-2xl font-semibold text-ink">裝置資訊</h2>
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
import { computed, onMounted, ref } from 'vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import BaseButton from '@/components/ui/BaseButton.vue';
import { useInitialState } from '@/lib/useInitialState';
import { fetchDeviceInfo } from '@/lib/services/deviceService';
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

const DEFAULT_VERSION = 'v1.0.0';
const DEFAULT_MASK = '******';

const normalizeDevice = (snapshot: Partial<DeviceInfo>): DeviceInfo => {
  const password = snapshot.password ?? '';
  const hasPassword = snapshot.has_password ?? Boolean(password);
  const masked =
    snapshot.masked_password ??
    (hasPassword && password.length > 0 ? '＊'.repeat(password.length) : DEFAULT_MASK);

  return {
    serial: snapshot.serial ?? 'RED-UNKNOWN',
    version: snapshot.version ?? DEFAULT_VERSION,
    status: snapshot.status ?? 0,
    password,
    masked_password: masked || DEFAULT_MASK,
    qrcode: snapshot.qrcode,
    has_password: hasPassword
  };
};

const info = ref<DeviceInfo>(normalizeDevice(initialDevice));
const passwordVisible = ref(false);

const qrcode = computed(() => info.value.qrcode ?? '');

// 依據顯示狀態切換密碼或遮罩文字
const displayPassword = computed(() => (passwordVisible.value ? info.value.password : info.value.masked_password));

// 切換密碼顯示與否
const togglePassword = () => {
  passwordVisible.value = !passwordVisible.value;
};

const loadDeviceInfo = async () => {
  try {
    const { device } = await fetchDeviceInfo();
    if (device) {
      info.value = normalizeDevice(device);
    }
  } catch (error) {
    console.error('取得裝置資訊失敗', error);
  }
};

onMounted(() => {
  void loadDeviceInfo();
});

// 建立隱藏下載連結，讓使用者保存 QRCode 圖檔
const downloadQr = () => {
  if (!qrcode.value) return;
  const link = document.createElement('a');
  link.href = qrcode.value;
  link.download = `device-${info.value.serial}.png`;
  link.click();
};
</script>
