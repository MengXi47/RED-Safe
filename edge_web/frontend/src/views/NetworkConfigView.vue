<template>
  <div class="space-y-8">
    <header class="page-hero">
      <h2 class="text-2xl font-semibold text-ink">網路配置</h2>
    </header>
    <div class="grid gap-6 xl:grid-cols-2">
      <BaseCard title="網路資訊">
        <dl class="grid gap-4 sm:grid-cols-2">
          <div v-for="item in networkItems" :key="item.label" class="rounded-xl bg-surface-subtle p-4">
            <dt class="text-xs text-ink-muted">{{ item.label }}</dt>
            <dd class="mt-1 text-base font-semibold text-ink">{{ item.value }}</dd>
          </div>
        </dl>
      </BaseCard>
      <BaseCard title="服務連接埠" :description="`偵測主機：${statusHost}`">
        <ul class="grid gap-4">
          <li
            v-for="port in portItems"
            :key="port.label"
            class="rounded-2xl border border-border bg-surface-default p-4 shadow-elev-sm"
          >
            <div class="flex items-center justify-between">
              <p class="text-sm font-semibold text-ink">{{ port.label }}</p>
              <span :class="statusBadge(port.state)">{{ port.state }}</span>
            </div>
            <p class="mt-2 text-sm text-ink-muted">{{ port.description }}</p>
          </li>
        </ul>
      </BaseCard>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed, onBeforeUnmount, onMounted, ref } from 'vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import type { NetworkInterfaceInfo, PortStatusResponse } from '@/types/network';
import { useInitialState } from '@/lib/useInitialState';
import { fetchPortStatus } from '@/lib/services/networkService';
import { useUiStore } from '@/store/ui';

/**
  * 組件用途：整合裝置網路設定與服務連接埠狀態。
  * 輸入參數：無，網路資訊透過 initial state 注入，Port 透過 API 取得。
  * 與其他模組關聯：使用 BaseCard 呈現資訊並透過 uiStore 通知錯誤。
  */

const initialNetwork = useInitialState<Partial<NetworkInterfaceInfo>>(
  (state) => state.network ?? {},
  () => ({})
);

const uiStore = useUiStore();
const status = ref<PortStatusResponse>();
const pollingTimer = ref<number | null>(null);
const hasNotifiedError = ref(false);

// 統一補齊缺漏欄位，避免畫面顯示空值與 undefined
const info = computed<NetworkInterfaceInfo>(() => ({
  ip_address: initialNetwork.ip_address ?? '--',
  netmask: initialNetwork.netmask ?? '--',
  gateway: initialNetwork.gateway ?? '--',
  dns: initialNetwork.dns ?? []
}));

// 將網路資訊轉換成陣列，方便以 v-for 呈現
const networkItems = computed(() => [
  { label: 'IP Address', value: info.value.ip_address },
  { label: '子網路遮罩', value: info.value.netmask },
  { label: '預設路由器', value: info.value.gateway },
  { label: 'DNS Server', value: info.value.dns.join(', ') || 'N/A' }
]);

const statusHost = computed(() => status.value?.host ?? '--');

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
