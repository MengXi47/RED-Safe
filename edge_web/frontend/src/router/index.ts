import { createRouter, createWebHistory, type RouteRecordRaw } from 'vue-router';

const routes: RouteRecordRaw[] = [
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
    path: '/user/bound',
    name: 'user-bound',
    component: () => import('@/views/UserBoundView.vue'),
    meta: { title: '已綁定的使用者', requiresAuth: true }
  },
  {
    path: '/network/ip',
    name: 'network-ip',
    component: () => import('@/views/NetworkIpView.vue'),
    meta: { title: '網路設定 - IP', requiresAuth: true }
  },
  {
    path: '/network/port',
    name: 'network-port',
    component: () => import('@/views/NetworkPortView.vue'),
    meta: { title: '網路設定 - Port', requiresAuth: true }
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
    path: '/settings',
    name: 'settings',
    component: () => import('@/views/SettingsView.vue'),
    meta: { title: '系統設定', requiresAuth: true }
  }
];

const router = createRouter({
  history: createWebHistory(),
  routes
});

router.beforeEach((to, _from, next) => {
  document.title = to.meta.title ? `RED Safe | ${String(to.meta.title)}` : 'RED Safe';
  // Placeholder for auth guard integration.
  if (to.meta.requiresAuth) {
    // TODO: integrate with backend session status
  }
  next();
});

export default router;
