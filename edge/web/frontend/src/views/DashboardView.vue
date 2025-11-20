<template>
  <div class="space-y-8">
    <section class="page-hero">
      <h2 class="text-2xl font-semibold text-ink">系統效能監控</h2>
    </section>
    <div class="grid gap-6 md:grid-cols-2 xl:grid-cols-4">
      <BaseCard title="CPU 使用率">
        <MetricProgress :label="'目前使用率'" :value="metrics?.cpu.percent ?? 0" />
        <MetricSparkline class="mt-4 text-brand-600" :points="cpuHistory" />
      </BaseCard>
      <BaseCard title="記憶體">
        <MetricProgress
          :label="`${metrics?.ram.used_mb ?? '--'} / ${metrics?.ram.total_mb ?? '--'} MB`"
          :value="metrics?.ram.percent ?? 0"
        />
        <MetricSparkline class="mt-4 text-brand-500" :points="memoryHistory" />
      </BaseCard>
      <BaseCard title="磁碟">
        <MetricProgress
          :label="`${metrics?.disk.used_gb ?? '--'} / ${metrics?.disk.total_gb ?? '--'} GB`"
          :value="metrics?.disk.percent ?? 0"
        />
        <MetricSparkline class="mt-4 text-brand-700" :points="diskHistory" />
      </BaseCard>
      <BaseCard title="設備溫度">
        <div class="flex h-full flex-col gap-4">
          <div class="flex flex-1 items-center justify-center text-2xl font-semibold text-brand-600">
            {{ metrics?.temperature ?? '--' }}
          </div>
          <MetricSparkline class="text-danger" :points="temperatureHistory" />
        </div>
      </BaseCard>
    </div>
  </div>
</template>

<script setup lang="ts">
import { onBeforeUnmount, onMounted, ref, type Ref } from 'vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import MetricProgress from '@/components/visual/MetricProgress.vue';
import MetricSparkline from '@/components/visual/MetricSparkline.vue';
import { fetchMetrics } from '@/lib/services/metricsService';
import type { MetricsResponse } from '@/types/metrics';
import { useUiStore } from '@/store/ui';

/**
  * 組件用途：呈現系統效能儀表板並定時更新 CPU、記憶體、磁碟與溫度資訊。
  * 輸入參數：無外部 props，依賴 metricsService 取得資料並使用 BaseCard 呈現。
  * 與其他模組關聯：透過 uiStore 顯示 Toast 通知錯誤狀態。
  */

const HISTORY_LIMIT = 24;

const metrics = ref<MetricsResponse>();
const uiStore = useUiStore();
const metricsTimer = ref<number | null>(null);
const cpuHistory = ref<number[]>([]);
const memoryHistory = ref<number[]>([]);
const diskHistory = ref<number[]>([]);
const temperatureHistory = ref<number[]>([]);

const pushHistory = (historyRef: Ref<number[]>, value: number | null) => {
  if (value === null || Number.isNaN(value)) {
    return;
  }
  const next = [...historyRef.value, value];
  historyRef.value = next.slice(-HISTORY_LIMIT);
};

// 抓取最新監控資料，失敗時透過 Toast 告知使用者
const loadMetrics = async () => {
  try {
    const response = await fetchMetrics();
    metrics.value = response;
    pushHistory(cpuHistory, response.cpu?.percent ?? null);
    pushHistory(memoryHistory, response.ram?.percent ?? null);
    pushHistory(diskHistory, response.disk?.percent ?? null);
    const tempValue = parseFloat(String(response.temperature ?? ''));
    pushHistory(temperatureHistory, Number.isFinite(tempValue) ? tempValue : null);
  } catch (error) {
    console.error(error);
    uiStore.pushToast('無法取得系統監控資料', 'danger');
  }
};

onMounted(() => {
  loadMetrics();
  metricsTimer.value = window.setInterval(loadMetrics, 5000);
});

onBeforeUnmount(() => {
  if (metricsTimer.value !== null) {
    window.clearInterval(metricsTimer.value);
    metricsTimer.value = null;
  }
});
</script>
