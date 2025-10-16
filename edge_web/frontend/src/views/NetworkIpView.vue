<template>
  <div class="space-y-8">
    <header>
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

const initial = (window as Window & { __EDGE_INITIAL_STATE__?: any }).__EDGE_INITIAL_STATE__?.network ?? {};

const info = computed<NetworkInterfaceInfo>(() => ({
  ip_address: initial.ip_address ?? '--',
  netmask: initial.netmask ?? '--',
  gateway: initial.gateway ?? '--',
  dns: initial.dns ?? []
}));

const networkItems = computed(() => [
  { label: 'IP Address', value: info.value.ip_address },
  { label: '子網路遮罩', value: info.value.netmask },
  { label: '預設路由器', value: info.value.gateway },
  { label: 'DNS Server', value: info.value.dns.join(', ') || 'N/A' }
]);
</script>
