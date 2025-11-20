<template>
  <div :class="isAuth ? '' : 'min-h-screen bg-surface-subtle text-ink'">
    <AppShell v-if="!isAuth" />
    <!-- 使用路由完整路徑作為 key，避免同一路由名稱快取導致畫面不更新 -->
    <RouterView v-else :key="route.fullPath" />
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue';
import { RouterView, useRoute } from 'vue-router';
import AppShell from '@/components/layout/AppShell.vue';

/**
  * 組件用途：根層判斷是否使用驗證版型，並在一般頁面包覆 AppShell。
  * 與其他模組關聯：依照路由 meta 切換 AuthLayout 或主框架。
  */

const route = useRoute();

// 判斷目前路由是否走認證流程，以套用不同的頁面容器
const isAuth = computed(() => route.meta.authLayout === true);
</script>
