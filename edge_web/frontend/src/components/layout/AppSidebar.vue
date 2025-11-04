<template>
  <aside
    :class="[
      'app-sidebar hidden w-72 flex-col border-r border-border bg-surface-default px-6 pb-8 pt-8 shadow-elev-md md:flex',
      { 'sidebar--collapsed': collapsed }
    ]"
    :data-collapsed="collapsed"
  >
    <RouterLink to="/" class="app-sidebar__brand mb-8 flex items-center gap-3">
      <div class="flex h-10 w-10 items-center justify-center rounded-full bg-brand-500 text-white">
        RS
      </div>
      <div class="app-sidebar__brand-text">
        <p class="text-sm font-semibold text-ink">RED Safe</p>
        <p class="text-xs text-ink-muted">Edge 控制台</p>
      </div>
    </RouterLink>
    <nav class="app-sidebar__nav flex-1 space-y-6">
      <div v-for="group in navGroups" :key="group.label">
        <p class="app-sidebar__section mb-2 text-xs font-semibold uppercase tracking-wide text-ink-muted">
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
    <div class="app-sidebar__bottom">
      <a
        class="sidebar-logout flex items-center gap-3 rounded-xl px-3 py-2 text-sm font-medium text-ink-muted"
        href="/logout/"
        aria-label="登出"
        @click.prevent="handleLogout"
      >
        <span class="sidebar-logout__icon" aria-hidden="true">
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none">
            <path
              d="M9 5H5a2 2 0 0 0-2 2v10a2 2 0 0 0 2 2h4M16 17l5-5-5-5M21 12H9"
              stroke="currentColor"
              stroke-width="1.6"
              stroke-linecap="round"
              stroke-linejoin="round"
            />
          </svg>
        </span>
        <span class="sidebar-logout__label">登出</span>
      </a>
      <footer class="app-sidebar__footer border-t border-border pt-6">
        <p class="text-xs text-ink-muted">&copy; {{ year }} RED Safe</p>
      </footer>
    </div>
  </aside>
</template>

<script setup lang="ts">
import { computed } from 'vue';
import { useRoute } from 'vue-router';

/**
  * 組件用途：呈現主側邊導覽並依據裝置狀態提供收合模式。
  * 輸入參數：collapsed 控制縮小版視圖，外層 AppShell 會傳入。
  * 與其他模組關聯：依賴 Router 判斷目前路徑並顯示不同群組連結。
  */

const props = withDefaults(
  defineProps<{
    collapsed?: boolean;
  }>(),
  { collapsed: false }
);

const collapsed = computed(() => Boolean(props.collapsed));

type NavItem = { to: string; label: string };
type NavGroup = { label: string; items: NavItem[] };

// 導覽資料：依功能群組整理路由，提供 template 迴圈使用
const navGroups: NavGroup[] = [
  {
    label: '總覽',
    items: [{ to: '/', label: '儀表板' }]
  },
  {
    label: '攝影機',
    items: [
      { to: '/cameras/live', label: '即時顯示' },
      { to: '/cameras', label: '攝影機管理' }
      
    ]
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

// 判斷連結是否為目前頁面或其子路徑，用於啟用態樣式
const isActive = (to: string) => route.path === to || route.path.startsWith(`${to}/`);

const handleLogout = () => {
  if (typeof window === 'undefined') return;
  if (window.confirm('確定要登出嗎？')) {
    window.location.href = '/logout/';
  }
};
</script>
