<template>
  <div class="space-y-8">
    <div class="space-y-6">
      <BaseCard title="">
        <div class="space-y-5">
          <div
            ref="streamShellRef"
            class="group relative aspect-[16/9] w-full overflow-hidden rounded-3xl border border-border/60 bg-ink-inverse shadow-elev-lg"
          >
            <video
              ref="videoRef"
              autoplay
              playsinline
              muted
              :class="['absolute inset-0 h-full w-full', videoFitClass]"
            ></video>

            <div class="pointer-events-none absolute left-4 top-4 flex items-center gap-2 text-xs font-semibold">
              <span class="rounded-full px-3 py-1 text-white" :class="statusBadge.class">{{ statusBadge.label }}</span>
              <span class="rounded-full bg-black/60 px-3 py-1 text-white/90">{{ playerResolutionLabel }}</span>
            </div>

            <div
              v-if="!hasStream"
              class="absolute inset-0 flex flex-col items-center justify-center gap-3 text-sm text-ink-muted"
            >
              <BaseLoadingSpinner size="sm" />
              <p>{{ connectionMessage }}</p>
            </div>
          </div>

          <div class="flex flex-wrap items-center gap-3">
            <BaseButton :loading="isConnecting" :disabled="isConnecting" @click="handleReconnect">
              {{ isConnecting ? '連線中...' : '重新連線' }}
            </BaseButton>
            <BaseButton variant="secondary" :disabled="!peerConnection" @click="handleDisconnect">
              中斷連線
            </BaseButton>
            <BaseButton variant="ghost" size="sm" @click="toggleFitMode">
              切換：{{ videoFitLabel }}
            </BaseButton>
            <p class="text-sm text-ink-muted">{{ statusHint }}</p>
          </div>
        </div>
      </BaseCard>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed, nextTick, onBeforeUnmount, onMounted, ref } from 'vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import BaseButton from '@/components/ui/BaseButton.vue';
import BaseLoadingSpinner from '@/components/ui/BaseLoadingSpinner.vue';
import { useUiStore } from '@/store/ui';

/**
  * 組件用途：與 Edge 上的 WebRTC signaling server 建立連線，播放 Mosaic 影像。
  * 輸入參數：無，Endpoint 透過 VITE_FALL_WEBRTC_URL 或預設為目前主機的 8765 埠 /webrtc/offer。
  * 與其他模組關聯：依賴 uiStore 顯示通知，並使用 BaseCard/BaseButton 呈現操作。
  */

const uiStore = useUiStore();
const videoRef = ref<HTMLVideoElement | null>(null);
const streamShellRef = ref<HTMLElement | null>(null);
const peerConnection = ref<RTCPeerConnection | null>(null);
const hasStream = ref(false);
const isConnecting = ref(false);
const connectionMessage = ref('等待串流來源...');
const lastKnownState = ref<'idle' | 'connected' | 'disconnected' | 'failed'>('idle');
const playerMetrics = ref({ width: 0, height: 0 });
const videoFitMode = ref<'contain' | 'cover'>('contain');
let resizeObserver: ResizeObserver | null = null;

const resolveSignalingUrl = (): string => {
  const configuredUrl = (import.meta.env.VITE_FALL_WEBRTC_URL ?? '').trim();
  if (configuredUrl) {
    return configuredUrl;
  }

  if (typeof window === 'undefined') {
    return 'http://127.0.0.1:8765/webrtc/offer';
  }

  const protocol = window.location.protocol === 'https:' ? 'https' : 'http';
  const host = window.location.hostname || '127.0.0.1';
  const envPort = (import.meta.env.VITE_FALL_WEBRTC_PORT ?? '').trim();
  const port = envPort || '8765';
  const portSegment = port ? `:${port}` : '';
  return `${protocol}://${host}${portSegment}/webrtc/offer`;
};

const signalingUrl = resolveSignalingUrl();

const statusHint = computed(() => {
  switch (lastKnownState.value) {
    case 'connected':
      return '串流已連線';
    case 'failed':
      return '連線失敗，請檢查裝置上的服務';
    case 'disconnected':
      return '串流已中斷，可嘗試重新連線';
    default:
      return '尚未建立連線';
  }
});

const statusBadge = computed(() => {
  const map: Record<'idle' | 'connected' | 'disconnected' | 'failed', { label: string; class: string }> = {
    connected: { label: '串流中', class: 'bg-success/80 text-white' },
    failed: { label: '連線失敗', class: 'bg-danger/80 text-white' },
    disconnected: { label: '已中斷', class: 'bg-warning/80 text-ink' },
    idle: { label: '等待串流', class: 'bg-black/50 text-white' }
  };
  return map[lastKnownState.value];
});

const playerResolutionLabel = computed(() => {
  const { width, height } = playerMetrics.value;
  if (!width || !height) return hasStream.value ? '載入影格中' : '待建立連線';
  return `${Math.round(width)}×${Math.round(height)}px`;
});

const videoFitClass = computed(() =>
  videoFitMode.value === 'contain' ? 'object-contain' : 'object-cover'
);

const videoFitLabel = computed(() =>
  videoFitMode.value === 'contain' ? '等比例縮放' : '填滿畫面'
);

const cleanupPeer = async () => {
  const pc = peerConnection.value;
  if (!pc) return;
  try {
    pc.ontrack = null;
    pc.onconnectionstatechange = null;
    pc.oniceconnectionstatechange = null;
    pc.getSenders().forEach((sender) => sender.track?.stop());
    pc.getReceivers().forEach((receiver) => receiver.track?.stop());
    pc.close();
  } catch (error) {
    console.warn('cleanup peer error', error);
  } finally {
    peerConnection.value = null;
  }
};

const handleDisconnect = async () => {
  await cleanupPeer();
  hasStream.value = false;
  connectionMessage.value = '串流已中斷';
  if (videoRef.value) {
    videoRef.value.srcObject = null;
  }
  lastKnownState.value = 'disconnected';
};

const bindPeerEvents = (pc: RTCPeerConnection) => {
  pc.ontrack = (event) => {
    const [stream] = event.streams;
    if (videoRef.value && stream) {
      videoRef.value.srcObject = stream;
      hasStream.value = true;
      connectionMessage.value = '串流已連線';
      lastKnownState.value = 'connected';
    }
  };
  pc.onconnectionstatechange = () => {
    if (pc.connectionState === 'failed') {
      lastKnownState.value = 'failed';
      connectionMessage.value = '連線失敗';
      uiStore.pushToast('連線失敗，請稍候重試', 'danger');
    }
    if (pc.connectionState === 'disconnected') {
      hasStream.value = false;
      lastKnownState.value = 'disconnected';
      connectionMessage.value = '連線中斷，等待重新連線';
    }
  };
  pc.oniceconnectionstatechange = () => {
    if (pc.iceConnectionState === 'failed') {
      lastKnownState.value = 'failed';
      connectionMessage.value = 'ICE 建立失敗';
    }
  };
};

const connectStream = async () => {
  if (isConnecting.value) return;
  await cleanupPeer();
  hasStream.value = false;
  isConnecting.value = true;
  connectionMessage.value = '建立連線中...';
  try {
    const pc = new RTCPeerConnection({
      iceServers: [{ urls: 'stun:stun.l.google.com:19302' }]
    });
    bindPeerEvents(pc);
    peerConnection.value = pc;

    const offer = await pc.createOffer({ offerToReceiveVideo: true });
    await pc.setLocalDescription(offer);

    const response = await fetch(signalingUrl, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ sdp: offer.sdp, type: offer.type })
    });

    if (!response.ok) {
      throw new Error(`Signaling 回應錯誤：${response.status}`);
    }

    const answer = await response.json();
    await pc.setRemoteDescription(answer);
    connectionMessage.value = '等待串流來源...';
  } catch (error) {
    console.error(error);
    uiStore.pushToast('建立連線時發生錯誤', 'danger');
    connectionMessage.value = '無法建立連線，請稍後再試';
    lastKnownState.value = 'failed';
    await cleanupPeer();
  } finally {
    isConnecting.value = false;
  }
};

const handleReconnect = () => {
  connectStream();
};

const toggleFitMode = () => {
  videoFitMode.value = videoFitMode.value === 'contain' ? 'cover' : 'contain';
};

const observePlayer = () => {
  if (resizeObserver) {
    resizeObserver.disconnect();
    resizeObserver = null;
  }
  if (!streamShellRef.value || typeof ResizeObserver === 'undefined') return;
  resizeObserver = new ResizeObserver((entries) => {
    const entry = entries[0];
    if (!entry) return;
    const { width, height } = entry.contentRect;
    playerMetrics.value = { width, height };
  });
  resizeObserver.observe(streamShellRef.value);
};

onMounted(() => {
  connectStream();
  nextTick(() => {
    observePlayer();
  });
});

onBeforeUnmount(() => {
  handleDisconnect();
  resizeObserver?.disconnect();
});
</script>
