<template>
  <div class="space-y-8">
    <header class="page-hero">
      <div class="flex flex-col gap-3 md:flex-row md:items-center md:justify-between">
        <div>
          <h2 class="text-2xl font-semibold text-ink">攝影機管理</h2>
          <p class="mt-1 text-sm text-ink-muted">搜尋周邊攝影機並管理已綁定的串流來源。</p>
        </div>
        <div class="flex items-center gap-3">
          <BaseButton :loading="scanning" @click="performScan">搜尋攝影機</BaseButton>
          <BaseButton variant="ghost" @click="refresh">重新整理</BaseButton>
        </div>
      </div>
    </header>

    <div class="grid gap-6 xl:grid-cols-2">
      <BaseCard title="搜尋結果" description="搜尋到的攝影機可以預覽串流或直接綁定">
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

      <BaseCard title="已綁定攝影機" description="綁定後即可提供 Edge 裝置串流">
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
      @close="previewOpen = false"
    />
  </div>
</template>

<script setup lang="ts">
import { computed, onBeforeUnmount, ref } from 'vue';
import BaseButton from '@/components/ui/BaseButton.vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import BaseTable, { type ColumnDefinition } from '@/components/ui/BaseTable.vue';
import CameraBindModal from '@/components/cameras/CameraBindModal.vue';
import CameraPreviewModal from '@/components/cameras/CameraPreviewModal.vue';
import { scanCameras, bindCamera, unbindCamera, previewProbe, previewOffer, previewHangup } from '@/lib/services/cameraService';
import type { Camera, CameraBindPayload } from '@/types/camera';
import { useUiStore } from '@/store/ui';
import { HttpError } from '@/lib/http';

const uiStore = useUiStore();

const initialState = (window as Window & { __EDGE_INITIAL_STATE__?: any }).__EDGE_INITIAL_STATE__?.cameras ?? {};

const searchResults = ref<Camera[]>(initialState.searchResults ?? []);
const boundCameras = ref<Camera[]>(initialState.bound ?? []);

const scanning = ref(false);
const searchError = ref('');
const bindModalOpen = ref(false);
const bindLoading = ref(false);
const unbindLoading = ref(false);
const selectedCamera = ref<Camera | null>(null);

const previewOpen = ref(false);
const previewSessionId = ref('');
const previewConnection = ref<RTCPeerConnection | null>(null);
const previewRef = ref<InstanceType<typeof CameraPreviewModal>>();
const previewCredentials = ref<{ account: string; password: string }>({ account: '', password: '' });

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

const performScan = async () => {
  scanning.value = true;
  searchError.value = '';
  try {
    const response = await scanCameras();
    if (!response.ok) {
      searchError.value = response.error ?? '掃描失敗';
      return;
    }
    searchResults.value = response.results;
  } catch (error) {
    console.error(error);
    searchError.value = '掃描失敗，請稍後再試';
  } finally {
    scanning.value = false;
  }
};

const prepareBind = (camera: Camera) => {
  selectedCamera.value = camera;
  bindModalOpen.value = true;
};

const confirmBind = async (payload: { custom_name: string; ipc_account?: string; ipc_password?: string }) => {
  if (!selectedCamera.value) return;
  bindLoading.value = true;
  try {
    const requestPayload: CameraBindPayload = {
      ip_address: selectedCamera.value.ip,
      mac_address: selectedCamera.value.mac,
      ipc_name: selectedCamera.value.name,
      custom_name: payload.custom_name || selectedCamera.value.name,
      ipc_account: payload.ipc_account,
      ipc_password: payload.ipc_password
    };
    const response = await bindCamera(requestPayload);
    if (response.ok && response.item) {
      updateBoundList(response.item);
      uiStore.pushToast('綁定成功', 'success');
      bindModalOpen.value = false;
    } else if (response.code === 'ALREADY_BOUND') {
      uiStore.pushToast('此攝影機已綁定', 'info');
      bindModalOpen.value = false;
    } else {
      uiStore.pushToast(response.error ?? '綁定失敗', 'danger');
    }
  } catch (error) {
    console.error(error);
    uiStore.pushToast('綁定失敗，請稍後再試', 'danger');
  } finally {
    bindLoading.value = false;
  }
};

const updateBoundList = (camera: Camera) => {
  const index = boundCameras.value.findIndex((item) => item.mac === camera.mac);
  if (index >= 0) {
    boundCameras.value.splice(index, 1, camera);
  } else {
    boundCameras.value.push(camera);
  }
  searchResults.value = searchResults.value.map((item) =>
    item.mac === camera.mac ? { ...item, is_bound: true } : item,
  );
};

const removeBind = async (camera: Camera) => {
  unbindLoading.value = true;
  try {
    const payload = camera.mac ? { mac_address: camera.mac } : { ip_address: camera.ip };
    const response = await unbindCamera(payload);
    if (response.ok) {
      boundCameras.value = boundCameras.value.filter((item) => item.mac !== camera.mac);
      searchResults.value = searchResults.value.map((item) =>
        item.mac === camera.mac ? { ...item, is_bound: false } : item,
      );
      uiStore.pushToast('已解除綁定', 'success');
    } else {
      uiStore.pushToast(response.error ?? '解除失敗', 'danger');
    }
  } catch (error) {
    console.error(error);
    uiStore.pushToast('解除失敗，請稍後再試', 'danger');
  } finally {
    unbindLoading.value = false;
  }
};

const refresh = () => window.location.reload();

const openPreview = async (camera: Camera) => {
  selectedCamera.value = camera;
  previewOpen.value = true;
};

const createPeerConnection = () => {
  const pc = new RTCPeerConnection({
    iceServers: [
      { urls: ['stun:stun.l.google.com:19302', 'stun:stun1.l.google.com:19302'] }
    ]
  });
  if (pc.addTransceiver) {
    pc.addTransceiver('video', { direction: 'recvonly' });
  }
  pc.addEventListener('track', (event) => {
    if (event.streams?.[0]) {
      previewRef.value?.attachStream(event.streams[0]);
    }
  });
  pc.addEventListener('iceconnectionstatechange', () => {
    if (pc.iceConnectionState === 'failed') {
      previewRef.value?.handleError('預覽連線失敗');
      stopPreview();
    }
  });
  return pc;
};

const promptCredentials = () => {
  const account = window.prompt('請輸入攝影機帳號', previewCredentials.value.account) ?? '';
  const password = window.prompt('請輸入攝影機密碼', previewCredentials.value.password) ?? '';
  previewCredentials.value = { account, password };
};

const startPreview = async (camera: Camera) => {
  previewCredentials.value = { account: '', password: '' };
  try {
    await previewProbe({ ip: camera.ip, account: '', password: '' });
  } catch (error: any) {
    if (error instanceof HttpError && (error.payload as any)?.code === 'AUTH_REQUIRED') {
      promptCredentials();
      if (!previewCredentials.value.account && !previewCredentials.value.password) {
        previewRef.value?.handleError('需要輸入攝影機帳號與密碼');
        return;
      }
      try {
        await previewProbe({
          ip: camera.ip,
          account: previewCredentials.value.account,
          password: previewCredentials.value.password
        });
      } catch (innerError: any) {
        previewRef.value?.handleError(innerError?.message ?? '驗證失敗');
        return;
      }
    } else {
      previewRef.value?.handleError(error?.message ?? '無法建立連線');
      return;
    }
  }

  try {
    const pc = createPeerConnection();
    previewConnection.value = pc;

    const offer = await pc.createOffer();
    await pc.setLocalDescription(offer);

    const payload = {
      ip: camera.ip,
      account: previewCredentials.value.account,
      password: previewCredentials.value.password,
      offer: {
        type: offer.type,
        sdp: offer.sdp
      }
    };

    const response = await previewOffer(payload);
    previewSessionId.value = response.session_id;
    await pc.setRemoteDescription(response.answer);
  } catch (error: any) {
    previewRef.value?.handleError(error?.message ?? '預覽失敗');
    stopPreview();
  }
};

const stopPreview = async () => {
  previewConnection.value?.getSenders().forEach((sender) => sender.track?.stop());
  previewConnection.value?.getReceivers().forEach((receiver) => receiver.track?.stop());
  previewConnection.value?.close();
  previewConnection.value = null;
  if (previewSessionId.value) {
    try {
      await previewHangup({ session_id: previewSessionId.value });
    } catch (error) {
      console.warn(error);
    }
  }
};

onBeforeUnmount(() => {
  stopPreview();
});
</script>
