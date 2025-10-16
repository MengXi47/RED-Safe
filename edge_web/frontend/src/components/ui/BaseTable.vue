<template>
  <div class="relative overflow-hidden rounded-2xl border border-slate-200 bg-white shadow-soft">
    <div class="max-h-[70vh] overflow-auto">
      <table class="min-w-full divide-y divide-slate-200">
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
        <tbody class="divide-y divide-slate-100">
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
          <tr v-else v-for="item in items" :key="itemKey(item)" class="hover:bg-brand-50">
            <td v-for="column in columns" :key="column.key" class="px-4 py-3 text-sm text-ink">
              <slot :name="`cell:${column.key}`" :item="item">
                {{ item[column.key as keyof typeof item] }}
              </slot>
            </td>
          </tr>
        </tbody>
      </table>
    </div>
    <footer v-if="showPagination" class="flex items-center justify-between border-t border-slate-200 p-4">
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

const totalPages = computed(() => Math.max(1, Math.ceil(props.totalItems / props.pageSize)));
const showPagination = computed(() => props.totalItems > props.pageSize);
const pageStart = computed(() => (props.page - 1) * props.pageSize + 1);
const pageEnd = computed(() => Math.min(props.totalItems, props.page * props.pageSize));
</script>
