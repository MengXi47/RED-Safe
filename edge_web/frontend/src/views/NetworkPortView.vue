<template>
  <div class="space-y-8">
    <header class="page-hero">
      <h2 class="text-2xl font-semibold text-ink">網路設定 - Port 狀態</h2>
      <p class="mt-1 text-sm text-ink-muted">定期輪詢常用服務的連接埠狀態，協助排查網路問題。</p>
    </header>
    <BaseCard title="服務連接埠" :description="`偵測主機：${status?.host ?? '--'}`">
      <ul class="grid gap-4 md:grid-cols-3">
        <li v-for="port in portItems" :key="port.label" class="rounded-2xl border border-border bg-surface-default p-4 shadow-elev-sm">
          <div class="flex items-center justify-between">
            <p class="text-sm font-semibold text-ink">{{ port.label }}</p>
            <span :class="statusBadge(port.state)">{{ port.state }}</span>
          </div>
          <p class="mt-2 text-sm text-ink-muted">{{ port.description }}</p>
        </li>
      </ul>
    </BaseCard>
  </div>
</template>

<script setup lang="ts">
import { computed, onBeforeUnmount, onMounted, ref } from 'vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import { fetchPortStatus } from '@/lib/services/networkService';
import type { PortStatusResponse } from '@/types/network';
import { useUiStore } from '@/store/ui';

/**
  * 組件用途：輪詢常見服務連接埠的狀態並以卡片顯示。
  * 輸入參數：無，靠後端 API 提供最新狀態。
  * 與其他模組關聯：使用 BaseCard、badge 樣式呈現狀態。
  */

const uiStore = useUiStore();
const status = ref<PortStatusResponse>();
const pollingTimer = ref<number | null>(null);
const hasNotifiedError = ref(false);

// 將後端狀態轉成簡潔的顯示資訊
const portItems = computed(() => {
  if (!status.value) {
    return [
      { label: 'HTTP', state: '未知', description: '尚未取得資料' },
      { label: 'HTTPS', state: '未知', description: '尚未取得資料' },
      { label: 'SSH', state: '未知', description: '尚未取得資料' }
    ];
  }
  const describe = (group: PortStatusResponse['http']) => {
    if (!group) return '無資料';
    if (group.ports?.length) {
      return `${group.listening ? '開啟於' : '偵測到'}：${group.ports.join(', ')}`;
    }
    return group.listening ? '服務已啟動' : '未開啟或未偵測到';
  };
  return [
    { label: 'HTTP', state: status.value.http.listening ? '運作中' : '未啟用', description: describe(status.value.http) },
    { label: 'HTTPS', state: status.value.https.listening ? '運作中' : '未啟用', description: describe(status.value.https) },
    { label: 'SSH', state: status.value.ssh.listening ? '運作中' : '未啟用', description: describe(status.value.ssh) }
  ];
});

// 依據狀態選擇不同的 badge 樣式
const statusBadge = (state: string) => {
  const base = 'badge';
  if (state === '運作中') {
    return `${base} badge-success`;
  }
  if (state === '未啟用') {
    return `${base} badge-muted`;
  }
  return `${base} badge-warning`;
};

// 呼叫 API 取得最新的連接埠狀態
const load = async () => {
  try {
    status.value = await fetchPortStatus();
    hasNotifiedError.value = false;
  } catch (error) {
    console.error(error);
    if (!hasNotifiedError.value) {
      uiStore.pushToast('無法取得連接埠狀態，請稍後再試', 'danger');
      hasNotifiedError.value = true;
    }
  }
};

onMounted(() => {
  load();
  pollingTimer.value = window.setInterval(load, 3000);
});

onBeforeUnmount(() => {
  if (pollingTimer.value !== null) {
    window.clearInterval(pollingTimer.value);
    pollingTimer.value = null;
  }
});
</script>
