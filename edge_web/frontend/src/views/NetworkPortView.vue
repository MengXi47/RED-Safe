<template>
  <div class="space-y-8">
    <header>
      <h2 class="text-2xl font-semibold text-ink">網路設定 - Port 狀態</h2>
      <p class="mt-1 text-sm text-ink-muted">定期輪詢常用服務的連接埠狀態，協助排查網路問題。</p>
    </header>
    <BaseCard title="服務連接埠" :description="`偵測主機：${status?.host ?? '--'}`">
      <ul class="grid gap-4 md:grid-cols-3">
        <li v-for="port in portItems" :key="port.label" class="rounded-2xl border border-slate-200 p-4">
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
import { computed, onMounted, ref } from 'vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import { fetchPortStatus } from '@/lib/services/networkService';
import type { PortStatusResponse } from '@/types/network';

const status = ref<PortStatusResponse>();

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

const statusBadge = (state: string) => {
  const base = 'rounded-full px-3 py-1 text-xs font-medium';
  if (state === '運作中') {
    return `${base} bg-green-100 text-green-700`;
  }
  if (state === '未啟用') {
    return `${base} bg-slate-100 text-ink-muted`;
  }
  return `${base} bg-amber-100 text-amber-700`;
};

const load = async () => {
  try {
    status.value = await fetchPortStatus();
  } catch (error) {
    console.error(error);
  }
};

onMounted(() => {
  load();
  const timer = window.setInterval(load, 3000);
  return () => window.clearInterval(timer);
});
</script>
