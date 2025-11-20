<template>
  <div class="space-y-6">
    <header class="flex flex-col gap-4 md:flex-row md:items-end md:justify-between">
      <div>
        <h1 class="text-2xl font-semibold text-ink">事件紀錄</h1>
      </div>
      <form class="flex flex-col gap-3 md:flex-row md:items-end" @submit.prevent="loadEvents">
        <label class="flex flex-col text-sm text-ink">
          <span class="text-ink-muted">開始時間</span>
          <input v-model="start" type="datetime-local" class="input" />
        </label>
        <label class="flex flex-col text-sm text-ink">
          <span class="text-ink-muted">結束時間</span>
          <input v-model="end" type="datetime-local" class="input" />
        </label>
        <button type="submit" class="btn btn-primary md:self-end">查詢</button>
      </form>
    </header>

    <div class="rounded-xl border border-border bg-surface-default shadow-elev-sm">
      <div class="overflow-x-auto">
        <table class="min-w-full text-left text-sm">
          <thead class="bg-surface-muted text-xs uppercase text-ink-muted">
            <tr>
              <th class="px-4 py-3">時間</th>
              <th class="px-4 py-3">事件類型</th>
              <th class="px-4 py-3">內容</th>
            </tr>
          </thead>
          <tbody>
            <tr v-for="ev in events" :key="ev.id" class="border-t border-border">
              <td class="px-4 py-3 whitespace-nowrap">{{ ev.occurred_at_display }}</td>
              <td class="px-4 py-3">{{ ev.event_type }}</td>
              <td class="px-4 py-3">
                <pre class="whitespace-pre-wrap text-xs text-ink-muted">{{ prettyPayload(ev.payload) }}</pre>
              </td>
            </tr>
            <tr v-if="!loading && events.length === 0">
              <td colspan="3" class="px-4 py-6 text-center text-sm text-ink-muted">目前沒有事件</td>
            </tr>
            <tr v-if="loading">
              <td colspan="3" class="px-4 py-6 text-center text-sm text-ink-muted">載入中…</td>
            </tr>
          </tbody>
        </table>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { onMounted, ref } from 'vue';
import { fetchEvents } from '@/lib/services/eventService';
import type { EdgeEventItem } from '@/types/event';

type EdgeEventView = EdgeEventItem & { occurred_at_display: string };

const events = ref<EdgeEventView[]>([]);
const loading = ref(false);
const start = ref<string>('');
const end = ref<string>('');

const pad2 = (n: number) => String(n).padStart(2, '0');

const formatLocalInput = (d: Date) => {
  const year = d.getFullYear();
  const month = pad2(d.getMonth() + 1);
  const day = pad2(d.getDate());
  const hour = pad2(d.getHours());
  const minute = pad2(d.getMinutes());
  return `${year}-${month}-${day}T${hour}:${minute}`;
};

const formatDisplay = (value?: string) => {
  if (!value) return '';
  const d = new Date(value);
  if (Number.isNaN(d.getTime())) return value;
  // 轉為 UTC，再套用固定 +8 時區
  const utcMs = d.getTime() + d.getTimezoneOffset() * 60 * 1000;
  const target = new Date(utcMs + 8 * 60 * 60 * 1000);
  const year = target.getFullYear();
  const month = pad2(target.getMonth() + 1);
  const day = pad2(target.getDate());
  const hour = pad2(target.getHours());
  const minute = pad2(target.getMinutes());
  return `${year}/${month}/${day} ${hour}:${minute}`;
};

const prettyPayload = (payload?: Record<string, unknown> | null) => {
  if (!payload || Object.keys(payload).length === 0) return '';
  try {
    return JSON.stringify(payload, null, 2);
  } catch (e) {
    return String(payload);
  }
};

async function loadEvents() {
  loading.value = true;
  try {
    const startParam = start.value ? `${start.value}+08:00` : undefined;
    const endParam = end.value ? `${end.value}+08:00` : undefined;
    const res = await fetchEvents({
      start: startParam,
      end: endParam
    });
    events.value =
      res.events?.map((ev) => ({
        ...ev,
        occurred_at_display: formatDisplay(ev.occurred_at)
      })) || [];
  } catch (e) {
    // TODO: 可以接 toast/通知
    console.error(e);
  } finally {
    loading.value = false;
  }
}

onMounted(() => {
  const now = new Date();
  const startDefault = new Date(now.getTime() - 24 * 60 * 60 * 1000);
  start.value = formatLocalInput(startDefault);
  end.value = formatLocalInput(now);
  loadEvents();
});
</script>

<style scoped>
.input {
  min-width: 220px;
  border: 1px solid var(--color-border, #e5e7eb);
  border-radius: 0.75rem;
  padding: 0.5rem 0.75rem;
  background: var(--color-surface-default, #fff);
}
.btn {
  border-radius: 0.75rem;
  padding: 0.5rem 1rem;
}
.btn-primary {
  background: #2563eb;
  color: #fff;
}
</style>
