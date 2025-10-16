<template>
  <aside
    class="hidden w-72 flex-col border-r border-slate-200 bg-white px-6 py-8 shadow-soft md:flex"
  >
    <RouterLink to="/" class="mb-8 flex items-center gap-3">
      <div class="flex h-10 w-10 items-center justify-center rounded-full bg-brand-500 text-white">
        RS
      </div>
      <div>
        <p class="text-sm font-semibold text-ink">RED Safe</p>
        <p class="text-xs text-ink-muted">Edge 控制台</p>
      </div>
    </RouterLink>
    <nav class="flex-1 space-y-6">
      <div v-for="group in navGroups" :key="group.label">
        <p class="mb-2 text-xs font-semibold uppercase tracking-wide text-ink-muted">
          {{ group.label }}
        </p>
        <ul class="space-y-1">
          <li v-for="item in group.items" :key="item.to">
            <RouterLink
              :to="item.to"
              class="flex items-center gap-3 rounded-xl px-3 py-2 text-sm font-medium text-ink-muted hover:bg-brand-50 hover:text-brand-600"
              :class="isActive(item.to) ? 'bg-brand-500/10 text-brand-600' : ''"
            >
              <span>{{ item.label }}</span>
            </RouterLink>
          </li>
        </ul>
      </div>
    </nav>
    <footer class="border-t border-slate-200 pt-6">
      <p class="text-xs text-ink-muted">&copy; {{ year }} RED Safe</p>
    </footer>
  </aside>
</template>

<script setup lang="ts">
import { useRoute } from 'vue-router';

const navGroups = [
  {
    label: '總覽',
    items: [{ to: '/', label: '儀表板' }]
  },
  {
    label: '攝影機',
    items: [{ to: '/cameras', label: '攝影機管理' }]
  },
  {
    label: '使用者',
    items: [{ to: '/user/bound', label: '已綁定使用者' }]
  },
  {
    label: '網路',
    items: [
      { to: '/network/ip', label: 'IP 設定' },
      { to: '/network/port', label: 'Port 狀態' }
    ]
  },
  {
    label: '裝置',
    items: [
      { to: '/device/info', label: '裝置資訊' },
      { to: '/device/change-password', label: '修改登入密碼' }
    ]
  },
  {
    label: '系統',
    items: [
      { to: '/logs', label: '系統日誌' },
      { to: '/settings', label: '系統設定' }
    ]
  }
];

const route = useRoute();
const year = new Date().getFullYear();

const isActive = (to: string) => route.path === to || route.path.startsWith(`${to}/`);
</script>
