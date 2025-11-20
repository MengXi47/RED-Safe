import { onScopeDispose, reactive, ref, shallowRef, type Ref } from 'vue';
import {
  bindCamera,
  fetchBoundCameras,
  previewHangup,
  previewOffer,
  previewProbe,
  scanCameras,
  unbindCamera
} from '@/lib/services/cameraService';
import type {
  Camera,
  CameraBindPayload,
  CameraBindResponse,
  CameraPreviewOfferPayload,
  CameraPreviewOfferResponse,
  CameraPreviewProbePayload,
  CameraUnbindResponse
} from '@/types/camera';
import { HttpError } from '@/lib/http';

type ToastVariant = 'success' | 'danger' | 'info';

interface Notifier {
  (message: string, variant?: ToastVariant): void;
}

interface CameraDiscoveryOptions {
  initialResults?: Camera[];
  notify?: Notifier;
}

interface CameraBindingOptions {
  initialBound?: Camera[];
  searchResults: Ref<Camera[]>;
  selectedCamera: Ref<Camera | null>;
  notify: Notifier;
}

interface CameraPreviewOptions {
  selectedCamera: Ref<Camera | null>;
  notify: Notifier;
  onStream: (stream: MediaStream) => void;
  onError: (message: string) => void;
}

const isAuthRequiredError = (error: unknown): error is HttpError<{ code?: string }> =>
  error instanceof HttpError && error.code === 'AUTH_REQUIRED';
const isTimeoutError = (error: unknown): error is HttpError<{ code?: string }> =>
  error instanceof HttpError && error.code === 'TIMEOUT';

/**
 * Hook 用途：管理攝影機掃描行為與錯誤提示。
 */
export const useCameraDiscovery = (options: CameraDiscoveryOptions = {}) => {
  const searchResults = ref<Camera[]>([...(options.initialResults ?? [])]);
  const scanning = ref(false);
  const searchError = ref('');

  const performScan = async () => {
    scanning.value = true;
    searchError.value = '';
    try {
      const response = await scanCameras();
      if (!response.ok) {
        const message = response.error ?? '掃描失敗';
        searchError.value = message;
        options.notify?.(message, 'danger');
        return;
      }
      searchResults.value = response.results;
    } catch (error) {
      console.error(error);
      const message = '掃描失敗，請稍後再試';
      searchError.value = message;
      options.notify?.(message, 'danger');
    } finally {
      scanning.value = false;
    }
  };

  return {
    searchResults,
    scanning,
    searchError,
    performScan
  };
};

/**
 * Hook 用途：集中處理攝影機綁定與解除邏輯。
 */
export const useCameraBinding = (options: CameraBindingOptions) => {
  const boundCameras = ref<Camera[]>([...(options.initialBound ?? [])]);
  const bindModalOpen = ref(false);
  const bindLoading = ref(false);
  const unbindLoading = ref(false);
  const boundRefreshing = ref(false);

  const syncSearchBoundState = () => {
    const boundMacSet = new Set(boundCameras.value.map((item) => item.mac));
    options.searchResults.value = options.searchResults.value.map((item) => ({
      ...item,
      is_bound: boundMacSet.has(item.mac)
    }));
  };

  const markSearchResult = (camera: Camera, isBound: boolean) => {
    options.searchResults.value = options.searchResults.value.map((item) =>
      item.mac === camera.mac ? { ...item, is_bound: isBound } : item
    );
  };

  const updateBoundList = (camera: Camera) => {
    const index = boundCameras.value.findIndex((item) => item.mac === camera.mac);
    if (index >= 0) {
      boundCameras.value.splice(index, 1, camera);
    } else {
      boundCameras.value.push(camera);
    }
    markSearchResult(camera, true);
  };

  const prepareBind = (camera: Camera) => {
    options.selectedCamera.value = camera;
    bindModalOpen.value = true;
  };

  const loadBoundCameras = async ({ silent = false }: { silent?: boolean } = {}) => {
    boundRefreshing.value = true;
    try {
      const response = await fetchBoundCameras();
      if (!response.ok) {
        if (!silent) {
          options.notify(response.error ?? '無法載入已綁定攝影機', 'danger');
        }
        return;
      }
      boundCameras.value = response.items ?? [];
      syncSearchBoundState();
    } catch (error) {
      console.error(error);
      if (!silent) {
        options.notify('無法載入已綁定攝影機', 'danger');
      }
    } finally {
      boundRefreshing.value = false;
    }
  };

  const confirmBind = async (payload: {
    custom_name: string;
    ipc_account?: string;
    ipc_password?: string;
  }) => {
    const camera = options.selectedCamera.value;
    if (!camera) return;
    if (typeof window !== 'undefined' && !window.confirm(`確認要綁定攝影機「${camera.name}」嗎？`)) {
      return;
    }
    bindLoading.value = true;
    try {
      const requestPayload: CameraBindPayload = {
        ip_address: camera.ip,
        mac_address: camera.mac,
        ipc_name: camera.name,
        custom_name: payload.custom_name || camera.name,
        ipc_account: payload.ipc_account,
        ipc_password: payload.ipc_password
      };
      const response: CameraBindResponse = await bindCamera(requestPayload);
      if (response.ok && response.item) {
        updateBoundList(response.item);
        options.notify('綁定成功', 'success');
        bindModalOpen.value = false;
        syncSearchBoundState();
      } else if (response.code === 'ALREADY_BOUND') {
        options.notify('此攝影機已綁定', 'info');
        bindModalOpen.value = false;
      } else {
        options.notify(response.error ?? '綁定失敗', 'danger');
      }
    } catch (error) {
      console.error(error);
      options.notify('綁定失敗，請稍後再試', 'danger');
    } finally {
      bindLoading.value = false;
    }
  };

  const removeBind = async (camera: Camera) => {
    if (typeof window !== 'undefined' && !window.confirm(`確認要解除攝影機「${camera.name}」的綁定嗎？`)) {
      return;
    }
    unbindLoading.value = true;
    try {
      const payload = camera.mac ? { mac_address: camera.mac } : { ip_address: camera.ip };
      const response: CameraUnbindResponse = await unbindCamera(payload);
      if (response.ok) {
        boundCameras.value = boundCameras.value.filter((item) => item.mac !== camera.mac);
        markSearchResult(camera, false);
        options.notify('已解除綁定', 'success');
        syncSearchBoundState();
      } else {
        options.notify(response.error ?? '解除失敗', 'danger');
      }
    } catch (error) {
      console.error(error);
      options.notify('解除失敗，請稍後再試', 'danger');
    } finally {
      unbindLoading.value = false;
    }
  };

  // 初始化時同步搜尋結果的綁定狀態，避免載入舊資料
  syncSearchBoundState();

  return {
    boundCameras,
    bindModalOpen,
    bindLoading,
    unbindLoading,
    boundRefreshing,
    prepareBind,
    confirmBind,
    removeBind,
    loadBoundCameras
  };
};


/**
 * Hook 用途：管理攝影機預覽的 WebRTC 連線與錯誤處理。
 */
export const useCameraPreview = (options: CameraPreviewOptions) => {
  const previewOpen = ref(false);
  const previewSessionId = ref('');
  const previewConnection = shallowRef<RTCPeerConnection | null>(null);
  const credentials = reactive({ account: '', password: '' });

  const resetSession = () => {
    previewSessionId.value = '';
    previewConnection.value = null;
    credentials.account = '';
    credentials.password = '';
  };

  const promptCredentials = () => {
    credentials.account = window.prompt('請輸入攝影機帳號', credentials.account) ?? '';
    credentials.password = window.prompt('請輸入攝影機密碼', credentials.password) ?? '';
  };

  const ensureProbe = async (camera: Camera) => {
    const payload: CameraPreviewProbePayload = {
      ip: camera.ip,
      account: '',
      password: ''
    };
    try {
      await previewProbe(payload);
      return;
    } catch (error) {
      if (!isAuthRequiredError(error)) {
        throw error;
      }
    }

    promptCredentials();
    if (!credentials.account && !credentials.password) {
      throw new HttpError('需要輸入攝影機帳號與密碼才能預覽', undefined, 'AUTH_REQUIRED');
    }

    await previewProbe({
      ip: camera.ip,
      account: credentials.account,
      password: credentials.password
    });
  };

  const createPeerConnection = () => {
    const pc = new RTCPeerConnection({
      iceServers: [{ urls: ['stun:stun.l.google.com:19302', 'stun:stun1.l.google.com:19302'] }]
    });
    if (pc.addTransceiver) {
      pc.addTransceiver('video', { direction: 'recvonly' });
    }
    pc.addEventListener('track', (event) => {
      const [stream] = event.streams ?? [];
      if (stream) {
        options.onStream(stream);
      }
    });
    pc.addEventListener('iceconnectionstatechange', () => {
      if (pc.iceConnectionState === 'failed') {
        options.onError('預覽連線失敗');
        void stopPreview();
      }
    });
    return pc;
  };

  const openPreview = (camera: Camera) => {
    options.selectedCamera.value = camera;
    previewOpen.value = true;
  };

  const startPreview = async (camera: Camera) => {
    credentials.account = '';
    credentials.password = '';
    try {
      await ensureProbe(camera);
    } catch (error) {
      const message = isTimeoutError(error)
        ? '攝影機連線逾時，請確認攝影機是否在線、帳密是否正確，或稍後再試'
        : error instanceof Error
          ? error.message
          : '無法建立連線';
      options.onError(message);
      return;
    }

    try {
      const pc = createPeerConnection();
      previewConnection.value = pc;

      const offer = await pc.createOffer();
      await pc.setLocalDescription(offer);

      const payload: CameraPreviewOfferPayload = {
        ip: camera.ip,
        account: credentials.account,
        password: credentials.password,
        offer: {
          type: offer.type,
          sdp: offer.sdp ?? ''
        }
      };

      const response: CameraPreviewOfferResponse = await previewOffer(payload);
      previewSessionId.value = response.session_id;
      await pc.setRemoteDescription(response.answer);
    } catch (error) {
      console.error(error);
      const message = isTimeoutError(error)
        ? '攝影機預覽逾時，請確認攝影機連線並稍後重試'
        : error instanceof Error
          ? error.message
          : '預覽失敗';
      options.onError(message);
      await stopPreview();
    }
  };

  const stopPreview = async () => {
    const connection = previewConnection.value;
    if (connection) {
      connection.getSenders().forEach((sender) => sender.track?.stop());
      connection.getReceivers().forEach((receiver) => receiver.track?.stop());
      connection.close();
    }
    previewConnection.value = null;
    const sessionId = previewSessionId.value;
    previewSessionId.value = '';
    if (sessionId) {
      try {
        await previewHangup({ session_id: sessionId });
      } catch (error) {
        console.warn('預覽結束通知失敗', error);
      }
    }
    resetSession();
  };

  const closePreview = () => {
    previewOpen.value = false;
    resetSession();
  };

  onScopeDispose(() => {
    void stopPreview();
  });

  return {
    previewOpen,
    previewSessionId,
    openPreview,
    startPreview,
    stopPreview,
    closePreview
  };
};
