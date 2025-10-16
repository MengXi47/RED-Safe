<template>
  <div class="space-y-8">
    <header class="page-hero">
      <h2 class="text-2xl font-semibold text-ink">已綁定的使用者</h2>
      <p class="mt-1 text-sm text-ink-muted">檢視與管理目前已綁定 Edge 裝置的帳號。</p>
    </header>
    <BaseCard>
      <BaseTable
        :columns="columns"
        :items="users"
        :loading="loading"
        :error="error"
        :total-items="users.length"
      >
        <template #cell:bind_at="{ item }">{{ formatDate(item.bind_at) }}</template>
        <template #cell:last_online="{ item }">{{ formatDate(item.last_online) }}</template>
        <template #cell:actions="{ item }">
          <BaseButton size="sm" variant="ghost" :loading="removing" @click="unbound(item.email)">
            解除綁定
          </BaseButton>
        </template>
      </BaseTable>
    </BaseCard>
  </div>
</template>

<script setup lang="ts">
import { onMounted, ref } from 'vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import BaseTable, { type ColumnDefinition } from '@/components/ui/BaseTable.vue';
import BaseButton from '@/components/ui/BaseButton.vue';
import { fetchBoundUsers, removeBoundUser } from '@/lib/services/userService';
import type { BoundUser } from '@/types/user';
import { useUiStore } from '@/store/ui';

const uiStore = useUiStore();
const initial = (window as Window & { __EDGE_INITIAL_STATE__?: any }).__EDGE_INITIAL_STATE__?.users ?? [];

const users = ref<BoundUser[]>(initial);
const loading = ref(false);
const removing = ref(false);
const error = ref('');

const columns: ColumnDefinition<BoundUser & { actions: string }>[] = [
  { key: 'email', label: 'Email' },
  { key: 'user_name', label: '使用者名稱' },
  { key: 'bind_at', label: '綁定時間' },
  { key: 'last_online', label: '最後上線' },
  { key: 'actions', label: '操作' }
];

const load = async () => {
  loading.value = true;
  error.value = '';
  try {
    const response = await fetchBoundUsers();
    users.value = response.items ?? [];
  } catch (err) {
    console.error(err);
    error.value = '無法載入使用者資料';
  } finally {
    loading.value = false;
  }
};

const unbound = async (email: string) => {
  if (!email) return;
  removing.value = true;
  try {
    const response = await removeBoundUser(email);
    if (response.status === 'ok') {
      users.value = users.value.filter((user) => user.email !== email);
      uiStore.pushToast('已解除綁定', 'success');
    } else {
      uiStore.pushToast(response.message ?? '解除失敗', 'danger');
    }
  } catch (err) {
    console.error(err);
    uiStore.pushToast('解除失敗，請稍後再試', 'danger');
  } finally {
    removing.value = false;
  }
};

const formatDate = (value?: string) => {
  if (!value) return '-';
  return new Intl.DateTimeFormat('zh-TW', {
    timeZone: 'Asia/Taipei',
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit'
  }).format(new Date(value));
};

onMounted(load);
</script>
