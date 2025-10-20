<template>
  <div class="space-y-8">
    <header class="page-hero">
      <h2 class="text-2xl font-semibold text-ink">網路設定 - IP</h2>
      <p class="mt-1 text-sm text-ink-muted">檢視裝置目前的網路參數，方便排查與設定。</p>
    </header>
    <BaseCard title="網路資訊">
      <dl class="grid gap-4 sm:grid-cols-2">
        <div v-for="item in networkItems" :key="item.label" class="rounded-xl bg-surface-subtle p-4">
          <dt class="text-xs text-ink-muted">{{ item.label }}</dt>
          <dd class="mt-1 text-base font-semibold text-ink">{{ item.value }}</dd>
        </div>
      </dl>
    </BaseCard>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import type { NetworkInterfaceInfo } from '@/types/network';
import { useInitialState } from '@/lib/useInitialState';

/**
  * 組件用途：顯示裝置目前的 IP、子網路與 DNS 設定。
  * 輸入參數：無，從伺服器注入的初始資料建立狀態。
  * 與其他模組關聯：透過 BaseCard 呈現重點資訊。
  */

const initialNetwork = useInitialState<Partial<NetworkInterfaceInfo>>(
  (state) => state.network ?? {},
  () => ({})
);

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
</script>
