<template>
  <div class="relative overflow-hidden rounded-2xl border border-border bg-surface-default shadow-elev-sm">
    <div class="max-h-[70vh] overflow-auto">
      <table class="min-w-full divide-y divide-border">
        <thead class="sticky top-0 bg-surface-subtle">
          <tr>
            <th
              v-for="column in columns"
              :key="column.key"
              scope="col"
              class="px-4 py-3 text-left text-xs font-semibold uppercase tracking-wide text-ink-muted"
            >
              <button
                v-if="column.sortable"
                type="button"
                class="flex items-center gap-1"
                @click="$emit('sort', column.key)"
              >
                {{ column.label }}
                <span v-if="sortKey === column.key">
                  <svg v-if="sortDir === 'asc'" class="h-3 w-3" viewBox="0 0 24 24" fill="none">
                    <path d="M7 14l5-5 5 5" stroke="currentColor" stroke-width="2" />
                  </svg>
                  <svg v-else class="h-3 w-3" viewBox="0 0 24 24" fill="none">
                    <path d="M7 10l5 5 5-5" stroke="currentColor" stroke-width="2" />
                  </svg>
                </span>
              </button>
              <span v-else>{{ column.label }}</span>
            </th>
          </tr>
        </thead>
        <tbody class="divide-y divide-border">
          <tr v-if="loading">
            <td :colspan="columns.length" class="px-4 py-10 text-center text-ink-muted">
              <BaseLoadingSpinner />
              <p class="mt-2 text-sm">載入中，請稍候...</p>
            </td>
          </tr>
          <tr v-else-if="error">
            <td :colspan="columns.length" class="px-4 py-10">
              <BaseEmpty :title="'載入失敗'" :description="error" icon="alert" />
            </td>
          </tr>
          <tr v-else-if="items.length === 0">
            <td :colspan="columns.length" class="px-4 py-10">
              <BaseEmpty title="目前沒有資料" description="請稍後再試或調整篩選條件。" />
            </td>
          </tr>
          <tr
            v-else
            v-for="(item, index) in items"
            :key="itemKey(item)"
            :class="[
              'transition-colors',
              index % 2 === 0 ? 'bg-surface-subtle/50' : 'bg-surface-default',
              'hover:bg-brand-50/60'
            ]"
          >
            <td v-for="column in columns" :key="column.key" class="px-4 py-3 text-sm text-ink">
              <slot :name="`cell:${column.key}`" :item="item">
                {{ item[column.key as keyof typeof item] }}
              </slot>
            </td>
          </tr>
        </tbody>
      </table>
    </div>
    <footer v-if="showPagination" class="flex items-center justify-between border-t border-border p-4">
      <span class="text-sm text-ink-muted">
        顯示第 {{ pageStart }} - {{ pageEnd }} 筆，共 {{ totalItems }} 筆
      </span>
      <div class="flex items-center gap-2">
        <BaseButton variant="ghost" size="sm" :disabled="page === 1" @click="$emit('update:page', page - 1)">
          上一頁
        </BaseButton>
        <span class="text-sm text-ink-muted">第 {{ page }} / {{ totalPages }} 頁</span>
        <BaseButton
          variant="ghost"
          size="sm"
          :disabled="page >= totalPages"
          @click="$emit('update:page', page + 1)"
        >
          下一頁
        </BaseButton>
      </div>
    </footer>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue';
import BaseLoadingSpinner from './BaseLoadingSpinner.vue';
import BaseEmpty from './BaseEmpty.vue';

/**
  * 組件用途：建立表格骨架並處理載入、錯誤與分頁狀態。
  * 輸入參數：columns 定義欄位、items 資料來源，另提供排序與分頁事件。
  * 與其他模組關聯：攝影機列表與已綁定清單共用此表格實作。
  */

export interface ColumnDefinition<T> {
  key: keyof T | string;
  label: string;
  sortable?: boolean;
}

const props = withDefaults(
  defineProps<{
    columns: ColumnDefinition<Record<string, unknown>>[];
    items: Record<string, unknown>[];
    itemKey?: (item: Record<string, unknown>) => string | number;
    loading?: boolean;
    error?: string;
    page?: number;
    pageSize?: number;
    totalItems?: number;
    sortKey?: string;
    sortDir?: 'asc' | 'desc';
  }>(),
  {
    items: () => [],
    itemKey: (item: Record<string, unknown>) => String(item.id ?? JSON.stringify(item)),
    loading: false,
    error: '',
    page: 1,
    pageSize: 10,
    totalItems: 0,
    sortKey: '',
    sortDir: 'asc'
  }
);

defineEmits(['update:page', 'sort']);

// 計算總頁數，至少為 1 以避免除以零
const totalPages = computed(() => Math.max(1, Math.ceil(props.totalItems / props.pageSize)));
// 控制是否顯示分頁區塊
const showPagination = computed(() => props.totalItems > props.pageSize);
// 目前頁面的起始/結束筆數，用於 footer 文案
const pageStart = computed(() => (props.page - 1) * props.pageSize + 1);
const pageEnd = computed(() => Math.min(props.totalItems, props.page * props.pageSize));
</script>
