<template>
  <div class="space-y-8">
    <header class="page-hero">
      <h2 class="text-2xl font-semibold text-ink">系統日誌</h2>
    </header>
    <BaseCard class="space-y-6">
      <div class="flex flex-col gap-4 sm:flex-row sm:items-center sm:justify-between">
        <p class="text-sm text-ink-muted">
          此頁每頁顯示 {{ pageSize }} 筆紀錄。
          <span v-if="maxRows" class="ml-1">系統最多保留 {{ maxRows }} 筆。</span>
        </p>
        <div class="flex items-center gap-2">
          <BaseButton variant="ghost" :loading="loading" @click="refresh">
            重新整理
          </BaseButton>
        </div>
      </div>
      <BaseTable
        :columns="columns"
        :items="presentedLogs"
        :loading="loading"
        :error="error"
        :page="page"
        :page-size="pageSize"
        :total-items="total"
        @update:page="handlePageChange"
      >
        <template #cell:created_at="{ item }">
          {{ formatDate(item.created_at) }}
        </template>
        <template #cell:details="{ item }">
          <div class="flex flex-col">
            <span class="font-medium text-ink">{{ item.action }}</span>
            <span class="mt-1 text-sm text-ink">{{ item.message }}</span>
            <span class="mt-2 text-xs text-ink-muted">
              分類：{{ item.category }}
              <template v-if="item.path"> · 路徑：{{ item.path }}</template>
            </span>
          </div>
        </template>
        <template #cell:source="{ item }">
          <div class="flex flex-col">
            <span class="font-medium text-ink">{{ item.ip || '未知來源' }}</span>
            <span v-if="item.userAgent" class="mt-1 max-w-[18rem] truncate text-xs text-ink-muted" :title="item.userAgent">
              {{ item.userAgent }}
            </span>
          </div>
        </template>
      </BaseTable>
    </BaseCard>
  </div>
</template>

<script setup lang="ts">
import { computed, onMounted, ref } from 'vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import BaseButton from '@/components/ui/BaseButton.vue';
import BaseTable, { type ColumnDefinition } from '@/components/ui/BaseTable.vue';
import { fetchSystemLogs } from '@/lib/services/logService';
import type { SystemLogEntry } from '@/types/log';
import { useUiStore } from '@/store/ui';

/**
  * 組件用途：查詢並呈現 Web 系統日誌，支援分頁刷新與自動裁切提示。
  * 與其他模組關聯：透過 logService 呼叫後端 API，並使用 BaseTable 呈現結果。
  */

type PresentedLog = SystemLogEntry & {
  ip: string;
  userAgent: string;
  path: string;
};

const uiStore = useUiStore();
const logs = ref<SystemLogEntry[]>([]);
const loading = ref(false);
const error = ref('');
const page = ref(1);
// 中文提示：預設每頁顯示 20 筆，如需調整可同步更新後端 API 的分頁參數。
const pageSize = 20;
const total = ref(0);
const maxRows = ref<number | null>(null);

const columns: ColumnDefinition<PresentedLog>[] = [
  { key: 'created_at', label: '時間' },
  { key: 'details', label: '事件內容' },
  { key: 'source', label: '來源資訊' }
];

const load = async () => {
  loading.value = true;
  error.value = '';
  try {
    const response = await fetchSystemLogs({
      limit: pageSize,
      offset: (page.value - 1) * pageSize
    });
    logs.value = response.items ?? [];
    total.value = response.total ?? 0;
    if (typeof response.max_rows === 'number') {
      maxRows.value = response.max_rows;
    }
  } catch (err) {
    console.error(err);
    error.value = '無法載入系統日誌';
    uiStore.pushToast('無法載入系統日誌', 'danger');
  } finally {
    loading.value = false;
  }
};

const refresh = () => {
  load();
};

const handlePageChange = (value: number) => {
  if (value === page.value) {
    load();
    return;
  }
  page.value = value;
  load();
};

const presentedLogs = computed<PresentedLog[]>(() =>
  logs.value.map((log) => {
    const metadata =
      log.metadata && typeof log.metadata === 'object' && !Array.isArray(log.metadata)
        ? (log.metadata as Record<string, unknown>)
        : {};
    const ip = typeof metadata.ip === 'string' ? metadata.ip : '';
    const userAgent = typeof metadata.user_agent === 'string' ? metadata.user_agent : '';
    const path = typeof metadata.path === 'string' ? metadata.path : '';
    return {
      ...log,
      ip,
      userAgent,
      path
    };
  })
);

const formatDate = (value: string) => {
  if (!value) return '-';
  const date = new Date(value);
  if (Number.isNaN(date.getTime())) return value;
  return new Intl.DateTimeFormat('zh-TW', {
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit'
  }).format(date);
};

onMounted(load);
</script>
