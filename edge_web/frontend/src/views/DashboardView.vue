<template>
  <div class="space-y-8">
    <section class="page-hero">
      <h2 class="text-2xl font-semibold text-ink">系統效能監控</h2>
      <p class="mt-1 text-sm text-ink-muted">即時掌握 Edge 裝置的 CPU、記憶體與磁碟使用情形。</p>
    </section>
    <div class="grid gap-6 md:grid-cols-2 xl:grid-cols-4">
      <BaseCard title="CPU 使用率">
        <MetricProgress :label="'目前使用率'" :value="metrics?.cpu.percent ?? 0" />
      </BaseCard>
      <BaseCard title="記憶體">
        <MetricProgress
          :label="`${metrics?.ram.used_mb ?? '--'} / ${metrics?.ram.total_mb ?? '--'} MB`"
          :value="metrics?.ram.percent ?? 0"
        />
      </BaseCard>
      <BaseCard title="磁碟">
        <MetricProgress
          :label="`${metrics?.disk.used_gb ?? '--'} / ${metrics?.disk.total_gb ?? '--'} GB`"
          :value="metrics?.disk.percent ?? 0"
        />
      </BaseCard>
      <BaseCard title="設備溫度">
        <div class="flex h-full items-center justify-center text-2xl font-semibold text-brand-600">
          {{ metrics?.temperature ?? '--' }}
        </div>
      </BaseCard>
    </div>
  </div>
</template>

<script setup lang="ts">
import { onMounted, ref } from 'vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import MetricProgress from '@/components/visual/MetricProgress.vue';
import { fetchMetrics } from '@/lib/services/metricsService';
import type { MetricsResponse } from '@/types/metrics';
import { useUiStore } from '@/store/ui';

const metrics = ref<MetricsResponse>();
const uiStore = useUiStore();

const loadMetrics = async () => {
  try {
    metrics.value = await fetchMetrics();
  } catch (error) {
    console.error(error);
    uiStore.pushToast('無法取得系統監控資料', 'danger');
  }
};

onMounted(() => {
  loadMetrics();
  const timer = window.setInterval(loadMetrics, 5000);
  return () => window.clearInterval(timer);
});
</script>
