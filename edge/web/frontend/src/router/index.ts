import { createRouter, createWebHistory, type RouteRecordRaw } from 'vue-router';

/**
 * 檔案用途：定義前端路由表與共用守門邏輯，維持頁面路徑與標題一致。
 * 存取規則：
 * - meta.authLayout：登入/註冊使用 AuthLayout，僅供未登入狀態進入。
 * - meta.requiresAuth：需經過權限驗證並套用 AppShell 佈局。
 * 與其他模組關聯：所有頁面透過動態匯入避免初次載入負擔。
 */

const routes: RouteRecordRaw[] = [
  {
    path: '/login',
    name: 'Login',
    component: () => import('@/views/LoginView.vue'),
    meta: { title: '登入系統', authLayout: true }
  },
  {
    path: '/setup',
    name: 'Register',
    component: () => import('@/views/LoginView.vue'),
    meta: { title: '首次使用：設定密碼', authLayout: true }
  },
  // {
  //   path: '/setup',
  //   redirect: { name: 'Register' }
  // },
  {
    path: '/',
    name: 'dashboard',
    component: () => import('@/views/DashboardView.vue'),
    meta: { title: '系統效能監控', requiresAuth: true }
  },
  {
    path: '/cameras',
    name: 'cameras',
    component: () => import('@/views/CamerasView.vue'),
    meta: { title: '攝影機管理', requiresAuth: true }
  },
  {
    path: '/cameras/detection',
    name: 'camera-detection',
    component: () => import('@/views/CameraDetectionView.vue'),
    meta: { title: '偵測功能', requiresAuth: true }
  },
  {
    path: '/cameras/live',
    name: 'camera-live',
    component: () => import('@/views/CameraRealtimeView.vue'),
    meta: { title: '即時顯示頁面', requiresAuth: true }
  },
  {
    path: '/user/bound',
    name: 'user-bound',
    component: () => import('@/views/UserBoundView.vue'),
    meta: { title: '已綁定的使用者', requiresAuth: true }
  },
  {
    path: '/network/config',
    name: 'network-config',
    component: () => import('@/views/NetworkConfigView.vue'),
    meta: { title: '網路配置', requiresAuth: true }
  },
  {
    path: '/device/info',
    name: 'device-info',
    component: () => import('@/views/DeviceInfoView.vue'),
    meta: { title: '裝置資訊', requiresAuth: true }
  },
  {
    path: '/device/change-password',
    name: 'device-change-password',
    component: () => import('@/views/DevicePasswordView.vue'),
    meta: { title: '修改登入密碼', requiresAuth: true }
  },
  {
    path: '/logs',
    name: 'logs',
    component: () => import('@/views/LogsView.vue'),
    meta: { title: '系統日誌', requiresAuth: true }
  },
  {
    path: '/events',
    name: 'events',
    component: () => import('@/views/EventsView.vue'),
    meta: { title: '事件紀錄', requiresAuth: true }
  },
  {
    path: '/settings',
    name: 'settings',
    component: () => import('@/views/SettingsView.vue'),
    meta: { title: '系統設定', requiresAuth: true }
  }
];

const router = createRouter({
  history: createWebHistory("/"),
  routes
});

// 進入路由前更新網頁標題並保留未來驗證流程掛載點
router.beforeEach((to, _from, next) => {
  document.title = to.meta.title ? `RED Safe | ${String(to.meta.title)}` : 'RED Safe';
  if (to.meta.requiresAuth) {
    // TODO：待與後端 Session 機制串接後補上實際驗證流程
  }
  next();
});

export default router;
