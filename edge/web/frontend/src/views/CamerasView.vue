<template>
  <div class="space-y-8">
    <header class="page-hero">
      <div class="flex flex-col gap-3 md:flex-row md:items-center md:justify-between">
        <div>
          <h2 class="text-2xl font-semibold text-ink">攝影機管理</h2>
          <p class="mt-1 text-sm text-ink-muted">搜尋周邊攝影機並管理已綁定的攝影機。</p>
        </div>
        <div class="flex items-center gap-3">
          <BaseButton :loading="scanning" @click="performScan">搜尋攝影機</BaseButton>
          <BaseButton variant="ghost" :loading="boundRefreshing || scanning" @click="refresh">重新整理</BaseButton>
        </div>
      </div>
    </header>

    <div class="grid gap-6 xl:grid-cols-2">
      <BaseCard title="搜尋結果">
        <BaseTable
          :columns="searchColumns"
          :items="searchResults"
          :loading="scanning"
          :error="searchError"
          :total-items="searchResults.length"
        >
          <template #cell:actions="{ item }">
            <div class="flex gap-2">
              <BaseButton size="sm" variant="ghost" @click="openPreview(item as Camera)">預覽</BaseButton>
              <BaseButton
                size="sm"
                variant="ghost"
                :disabled="(item as Camera).is_bound"
                @click="prepareBind(item as Camera)"
              >
                {{ (item as Camera).is_bound ? '已綁定' : '綁定' }}
              </BaseButton>
            </div>
          </template>
        </BaseTable>
      </BaseCard>

      <BaseCard title="已綁定攝影機">
        <BaseTable
          :columns="boundColumns"
          :items="boundCameras"
          :total-items="boundCameras.length"
        >
          <template #cell:actions="{ item }">
            <BaseButton size="sm" variant="ghost" @click="removeBind(item as Camera)" :loading="unbindLoading">
              解除綁定
            </BaseButton>
          </template>
        </BaseTable>
      </BaseCard>
    </div>

    <CameraBindModal
      :open="bindModalOpen"
      :camera="selectedCamera"
      :loading="bindLoading"
      @close="bindModalOpen = false"
      @submit="confirmBind"
    />

    <CameraPreviewModal
      ref="previewRef"
      :open="previewOpen"
      :camera="selectedCamera"
      :session-id="previewSessionId"
      @start="startPreview"
      @stop="stopPreview"
      @close="closePreview"
    />
  </div>
</template>

<script setup lang="ts">
import { onActivated, onMounted, ref } from 'vue';
import BaseButton from '@/components/ui/BaseButton.vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import BaseTable, { type ColumnDefinition } from '@/components/ui/BaseTable.vue';
import CameraBindModal from '@/components/cameras/CameraBindModal.vue';
import CameraPreviewModal from '@/components/cameras/CameraPreviewModal.vue';
import type { Camera } from '@/types/camera';
import { useUiStore } from '@/store/ui';
import { useInitialState } from '@/lib/useInitialState';
import {
  useCameraBinding,
  useCameraDiscovery,
  useCameraPreview
} from '@/composables/useCameraManagement';
import type { CamerasBootstrapState } from '@/types/bootstrap';

/**
  * 組件用途：整合攝影機掃描、綁定與 WebRTC 預覽的管理頁面。
  * 輸入參數：無外部 props，透過服務層呼叫後端 API 與展示結果。
  * 與其他模組關聯：使用 BaseTable 呈現列表，並搭配 CameraBindModal、CameraPreviewModal。
  */

const uiStore = useUiStore();

const initialState = useInitialState<CamerasBootstrapState>(
  (state) => state.cameras ?? {},
  () => ({})
);

const notify = (message: string, variant: 'success' | 'danger' | 'info' = 'info') => {
  uiStore.pushToast(message, variant);
};

const selectedCamera = ref<Camera | null>(null);

const {
  searchResults,
  scanning,
  searchError,
  performScan
} = useCameraDiscovery({
  initialResults: initialState.searchResults ?? [],
  notify
});

const {
  boundCameras,
  bindModalOpen,
  bindLoading,
  unbindLoading,
  boundRefreshing,
  prepareBind,
  confirmBind,
  removeBind,
  loadBoundCameras
} = useCameraBinding({
  initialBound: initialState.bound ?? [],
  searchResults,
  selectedCamera,
  notify
});

const previewRef = ref<InstanceType<typeof CameraPreviewModal>>();
const {
  previewOpen,
  previewSessionId,
  openPreview,
  startPreview,
  stopPreview,
  closePreview
} = useCameraPreview({
  selectedCamera,
  notify,
  onStream: (stream) => previewRef.value?.attachStream(stream),
  onError: (message) => previewRef.value?.handleError(message)
});

onMounted(() => {
  void loadBoundCameras({ silent: true });
});

onActivated(() => {
  void loadBoundCameras({ silent: true });
});

const searchColumns: ColumnDefinition<Camera & { actions: string }>[] = [
  { key: 'ip', label: 'IP 位址' },
  { key: 'mac', label: 'MAC 位址' },
  { key: 'name', label: '名稱' },
  { key: 'actions', label: '操作' }
];

const boundColumns: ColumnDefinition<Camera & { actions: string }>[] = [
  { key: 'ip', label: 'IP 位址' },
  { key: 'mac', label: 'MAC 位址' },
  { key: 'name', label: '名稱' },
  { key: 'actions', label: '操作' }
];

const refresh = async () => {
  await loadBoundCameras();
  await performScan();
};
</script>
