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
import { useInitialState } from '@/lib/useInitialState';

/**
  * 組件用途：列出已綁定的使用者並提供解除綁定操作。
  * 輸入參數：無，透過 userService 取得與更新資料。
  * 與其他模組關聯：使用 uiStore 提示操作結果、BaseTable 呈現表格。
  */

const uiStore = useUiStore();
const initialUsers = useInitialState<BoundUser[]>(
  (state) => state.users ?? [],
  () => []
);

const users = ref<BoundUser[]>(initialUsers);
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

// 取回綁定使用者列表並處理錯誤提示
const load = async () => {
  loading.value = true;
  error.value = '';
  try {
    const response = await fetchBoundUsers();
    if (response.error) {
      error.value = response.error;
      uiStore.pushToast(response.error, 'danger');
      return;
    }
    users.value = response.items ?? [];
  } catch (err) {
    console.error(err);
    error.value = '無法載入使用者資料';
    uiStore.pushToast('無法載入使用者資料', 'danger');
  } finally {
    loading.value = false;
  }
};

// 向後端發送解除綁定請求，成功後從列表移除
const unbound = async (email: string) => {
  if (!email) return;
  if (typeof window !== 'undefined' && !window.confirm(`確認要解除使用者 ${email} 的綁定嗎？`)) {
    return;
  }
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

// 將時間字串格式化為使用者可讀的本地時間
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
