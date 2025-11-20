<template>
  <div class="space-y-8">
    <header class="page-hero">
      <h2 class="text-2xl font-semibold text-ink">網路配置</h2>
    </header>

    <BaseCard title="網路資訊與設定">
      <div class="rounded-2xl border border-border px-4 py-5">
        <div class="space-y-4">
          <div class="grid grid-cols-3 items-center gap-3">
            <p class="text-sm text-ink-muted">模式</p>
            <div class="col-span-2">
              <template v-if="canConfigure">
                <select
                  class="w-full rounded-xl border border-border bg-surface-default px-3 py-2 text-sm text-ink focus:outline-none focus:ring-2 focus:ring-brand-400 disabled:cursor-not-allowed disabled:opacity-60"
                  :disabled="isSwitchingDhcp"
                  v-model="desiredMode"
                >
                  <option v-for="option in modeOptions" :key="option.value" :value="option.value">
                    {{ option.label }}
                  </option>
                </select>
              </template>
              <template v-else>
                <p class="text-sm font-medium text-ink">{{ displayModeLabel }}</p>
              </template>
            </div>
          </div>

          <div v-if="canConfigure" class="space-y-3">
            <div class="grid grid-cols-3 items-center gap-3">
              <p class="text-sm text-ink-muted">IP 地址</p>
              <div class="col-span-2">
                <template v-if="isManualSelected">
                  <input
                    v-model="manualForm.ip_address"
                    placeholder="192.168.0.10"
                    class="w-full rounded-xl border border-border px-3 py-2 text-sm text-ink focus:border-brand-500 focus:outline-none focus:ring-2 focus:ring-brand-200"
                  />
                  <p v-if="manualErrors.ip_address" class="mt-1 text-xs text-danger">{{ manualErrors.ip_address }}</p>
                </template>
                <template v-else>
                  <p class="text-sm font-medium text-ink">{{ networkInfo.ip_address }}</p>
                </template>
              </div>
            </div>

            <div class="grid grid-cols-3 items-center gap-3">
              <p class="text-sm text-ink-muted">掩碼</p>
              <div class="col-span-2">
                <template v-if="isManualSelected">
                  <input
                    v-model="manualForm.netmask"
                    placeholder="255.255.255.0"
                    class="w-full rounded-xl border border-border px-3 py-2 text-sm text-ink focus:border-brand-500 focus:outline-none focus:ring-2 focus:ring-brand-200"
                  />
                  <p v-if="manualErrors.netmask" class="mt-1 text-xs text-danger">{{ manualErrors.netmask }}</p>
                </template>
                <template v-else>
                    <p class="text-sm font-medium text-ink">{{ networkInfo.netmask }}</p>
                </template>
              </div>
            </div>

            <div class="grid grid-cols-3 items-center gap-3">
              <p class="text-sm text-ink-muted">網關</p>
              <div class="col-span-2">
                <template v-if="isManualSelected">
                  <input
                    v-model="manualForm.gateway"
                    placeholder="192.168.0.1"
                    class="w-full rounded-xl border border-border px-3 py-2 text-sm text-ink focus:border-brand-500 focus:outline-none focus:ring-2 focus:ring-brand-200"
                  />
                  <p v-if="manualErrors.gateway" class="mt-1 text-xs text-danger">{{ manualErrors.gateway }}</p>
                </template>
                <template v-else>
                    <p class="text-sm font-medium text-ink">{{ networkInfo.gateway }}</p>
                </template>
              </div>
            </div>

            <template v-if="isManualSelected">
              <div class="grid grid-cols-3 items-center gap-3">
                <p class="text-sm text-ink-muted">首選 DNS</p>
                <div class="col-span-2">
                  <input
                    v-model="manualForm.primaryDns"
                    placeholder="8.8.8.8"
                    class="w-full rounded-xl border border-border px-3 py-2 text-sm text-ink focus:border-brand-500 focus:outline-none focus:ring-2 focus:ring-brand-200"
                  />
                  <p v-if="manualErrors.primaryDns" class="mt-1 text-xs text-danger">{{ manualErrors.primaryDns }}</p>
                </div>
              </div>
              <div class="grid grid-cols-3 items-center gap-3">
                <p class="text-sm text-ink-muted">備用 DNS</p>
                <div class="col-span-2">
                  <input
                    v-model="manualForm.secondaryDns"
                    placeholder="1.1.1.1"
                    class="w-full rounded-xl border border-border px-3 py-2 text-sm text-ink focus:border-brand-500 focus:outline-none focus:ring-2 focus:ring-brand-200"
                  />
                  <p v-if="manualErrors.secondaryDns" class="mt-1 text-xs text-danger">{{ manualErrors.secondaryDns }}</p>
                </div>
              </div>
            </template>
            <template v-else>
              <div class="grid grid-cols-3 items-center gap-3">
                <p class="text-sm text-ink-muted">DNS</p>
                  <p class="col-span-2 text-sm font-medium text-ink">{{ networkInfo.dns.join(', ') || 'N/A' }}</p>
              </div>
            </template>
          </div>

          <div v-else class="space-y-3">
            <div class="grid grid-cols-3 items-center gap-3">
              <p class="text-sm text-ink-muted">IP 地址</p>
              <p class="col-span-2 text-sm font-medium text-ink">{{ networkInfo.ip_address }}</p>
            </div>
            <div class="grid grid-cols-3 items-center gap-3">
              <p class="text-sm text-ink-muted">掩碼</p>
              <p class="col-span-2 text-sm font-medium text-ink">{{ networkInfo.netmask }}</p>
            </div>
            <div class="grid grid-cols-3 items-center gap-3">
              <p class="text-sm text-ink-muted">網關</p>
              <p class="col-span-2 text-sm font-medium text-ink">{{ networkInfo.gateway }}</p>
            </div>
            <div class="grid grid-cols-3 items-center gap-3">
              <p class="text-sm text-ink-muted">DNS</p>
              <p class="col-span-2 text-sm font-medium text-ink">{{ networkInfo.dns.join(', ') || 'N/A' }}</p>
            </div>
          </div>

          <div class="pt-2">
            <template v-if="canConfigure">
              <BaseButton :loading="isSaving" :disabled="saveDisabled" @click="submitNetworkConfig">
                保存
              </BaseButton>
            </template>
            <template v-else>
              <p class="text-sm text-ink-muted">
                本頁面於 macOS/Windows 僅供檢視，調整 IP 請於 Linux 裝置上操作。
              </p>
            </template>
          </div>
        </div>
      </div>
    </BaseCard>

    <BaseCard title="服務連接埠" :description="`偵測主機：${statusHost}`">
      <ul class="grid gap-4">
        <li
          v-for="port in portItems"
          :key="port.label"
          class="rounded-2xl border border-border bg-surface-default p-4 shadow-elev-sm"
        >
          <div class="flex items-center justify-between">
            <p class="text-sm font-semibold text-ink">{{ port.label }}</p>
            <span :class="statusBadge(port.state)">{{ port.state }}</span>
          </div>
          <p class="mt-2 text-sm text-ink-muted">{{ port.description }}</p>
        </li>
      </ul>
    </BaseCard>
  </div>
</template>

<script setup lang="ts">
import { computed, onBeforeUnmount, onMounted, reactive, ref, watch } from 'vue';
import BaseButton from '@/components/ui/BaseButton.vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import type {
  NetworkCapabilities,
  NetworkInterfaceInfo,
  NetworkConfigUpdatePayload,
  NetworkConfigUpdateResponse,
  PortStatusResponse
} from '@/types/network';
import { HttpError } from '@/lib/http';
import { useInitialState } from '@/lib/useInitialState';
import { fetchNetworkInfo, fetchPortStatus, updateNetworkConfig } from '@/lib/services/networkService';
import { useUiStore } from '@/store/ui';

/**
  * 組件用途：整合裝置網路設定與服務連接埠狀態。
  * 輸入參數：無，網路資訊透過 initial state 注入並額外輪詢 API，Port 透過 API 取得。
  * 與其他模組關聯：使用 BaseCard 呈現資訊並透過 uiStore 通知錯誤。
  */

const initialNetwork = useInitialState<Partial<NetworkInterfaceInfo>>(
  (state) => state.network ?? {},
  () => ({})
);

const initialCapabilities = useInitialState<NetworkCapabilities>(
  (state) => state.networkCapabilities ?? { canConfigure: false },
  () => ({ canConfigure: false })
);

const shapeNetworkInfo = (input: Partial<NetworkInterfaceInfo> = {}): NetworkInterfaceInfo => ({
  interface_name: input.interface_name ?? '--',
  ip_address: input.ip_address ?? '--',
  netmask: input.netmask ?? '--',
  gateway: input.gateway ?? '--',
  dns: Array.isArray(input.dns) ? input.dns : [],
  mode: input.mode ?? 'UNAVAILABLE',
  mode_raw: input.mode_raw ?? 'NETWORK_MODE_UNSPECIFIED'
});

const sanitizeField = (value: string) => {
  if (!value || value === '--' || value === 'N/A') return '';
  return value;
};

const uiStore = useUiStore();
const networkInfo = ref<NetworkInterfaceInfo>(shapeNetworkInfo(initialNetwork));
const status = ref<PortStatusResponse>();
const portPollingTimer = ref<number | null>(null);
const hasNotifiedPortError = ref(false);
const hasNotifiedNetworkError = ref(false);
const isApplyingManual = ref(false);
const isSwitchingDhcp = ref(false);
const canConfigure = computed(() => Boolean(initialCapabilities.canConfigure));
const isDhcpMode = computed(() => networkInfo.value.mode?.toUpperCase() === 'DHCP');
const displayModeLabel = computed(() =>
  networkInfo.value.mode?.toUpperCase() === 'DHCP' ? '自動取得' : '靜態 IP'
);
const desiredMode = ref<'DHCP' | 'MANUAL'>(isDhcpMode.value ? 'DHCP' : 'MANUAL');
const isManualSelected = computed(() => desiredMode.value === 'MANUAL');
const modeOptions = [
  { label: '自動取得', value: 'DHCP' },
  { label: '靜態 IP', value: 'MANUAL' }
];

const interfaceName = ref(sanitizeField(networkInfo.value.interface_name));

const manualForm = reactive({
  ip_address: '',
  netmask: '',
  gateway: '',
  primaryDns: '',
  secondaryDns: ''
});

const manualErrors = reactive<Record<'ip_address' | 'netmask' | 'gateway' | 'primaryDns' | 'secondaryDns', string>>({
  ip_address: '',
  netmask: '',
  gateway: '',
  primaryDns: '',
  secondaryDns: ''
});

const buildDnsList = () =>
  [manualForm.primaryDns, manualForm.secondaryDns].map((entry) => entry.trim()).filter(Boolean);

const isSaving = computed(() => isApplyingManual.value || isSwitchingDhcp.value);
const saveDisabled = computed(() => {
  if (!canConfigure.value || isSaving.value) return true;
  if (!interfaceName.value || interfaceName.value === 'N/A') return true;
  if (!isManualSelected.value) {
    return false;
  }
  const dnsList = buildDnsList();
  return (
    !manualForm.ip_address.trim() ||
    !manualForm.netmask.trim() ||
    !manualForm.gateway.trim() ||
    !dnsList.length
  );
});

const resetManualErrors = () => {
  manualErrors.ip_address = '';
  manualErrors.netmask = '';
  manualErrors.gateway = '';
  manualErrors.primaryDns = '';
  manualErrors.secondaryDns = '';
};

const syncManualForm = () => {
  interfaceName.value = sanitizeField(networkInfo.value.interface_name);
  manualForm.ip_address = sanitizeField(networkInfo.value.ip_address);
  manualForm.netmask = sanitizeField(networkInfo.value.netmask);
  manualForm.gateway = sanitizeField(networkInfo.value.gateway);
  manualForm.primaryDns = sanitizeField(networkInfo.value.dns[0] || '');
  manualForm.secondaryDns = sanitizeField(networkInfo.value.dns[1] || '');
};

syncManualForm();

watch(
  () => ({ ...networkInfo.value }),
  () => {
    syncManualForm();
  }
);

watch(
  () => networkInfo.value.mode,
  (mode) => {
    desiredMode.value = mode?.toUpperCase() === 'DHCP' ? 'DHCP' : 'MANUAL';
  }
);

watch(desiredMode, () => {
  resetManualErrors();
});


const statusHost = computed(() => status.value?.host ?? '--');

// 將後端狀態轉成簡潔的顯示資訊
const portItems = computed(() => {
  if (!status.value) {
    return [
      { label: 'HTTP', state: '未知', description: '尚未取得資料' },
      { label: 'HTTPS', state: '未知', description: '尚未取得資料' },
      { label: 'SSH', state: '未知', description: '尚未取得資料' }
    ];
  }
  const describe = (group: PortStatusResponse['http']) => {
    if (!group) return '無資料';
    if (group.ports?.length) {
      return `${group.listening ? '開啟於' : '偵測到'}：${group.ports.join(', ')}`;
    }
    return group.listening ? '服務已啟動' : '未開啟或未偵測到';
  };
  return [
    { label: 'HTTP', state: status.value.http.listening ? '運作中' : '未啟用', description: describe(status.value.http) },
    { label: 'HTTPS', state: status.value.https.listening ? '運作中' : '未啟用', description: describe(status.value.https) },
    { label: 'SSH', state: status.value.ssh.listening ? '運作中' : '未啟用', description: describe(status.value.ssh) }
  ];
});

// 依據狀態選擇不同的 badge 樣式
const statusBadge = (state: string) => {
  const base = 'badge';
  if (state === '運作中') {
    return `${base} badge-success`;
  }
  if (state === '未啟用') {
    return `${base} badge-muted`;
  }
  return `${base} badge-warning`;
};

const loadPorts = async () => {
  try {
    status.value = await fetchPortStatus();
    hasNotifiedPortError.value = false;
  } catch (error) {
    console.error(error);
    if (!hasNotifiedPortError.value) {
      uiStore.pushToast('無法取得連接埠狀態，請稍後再試', 'danger');
      hasNotifiedPortError.value = true;
    }
  }
};

const loadNetwork = async () => {
  try {
    const data = await fetchNetworkInfo();
    networkInfo.value = shapeNetworkInfo(data);
    syncManualForm();
    hasNotifiedNetworkError.value = false;
  } catch (error) {
    console.error(error);
    if (!hasNotifiedNetworkError.value) {
      uiStore.pushToast('無法取得網路資訊，顯示為上次載入資料', 'info');
      hasNotifiedNetworkError.value = true;
    }
  }
};

const applyFieldErrors = (fields?: Record<string, string>) => {
  if (!fields) return;
  manualErrors.ip_address = fields.ip_address ?? manualErrors.ip_address;
  manualErrors.netmask = fields.netmask ?? manualErrors.netmask;
  manualErrors.gateway = fields.gateway ?? manualErrors.gateway;
  if (fields.dns) {
    manualErrors.primaryDns = fields.dns;
  }
};

const applyManualConfig = async (): Promise<boolean> => {
  if (!canConfigure.value || isApplyingManual.value) return false;
  resetManualErrors();

  const iface = interfaceName.value.trim();
  if (!iface) {
    uiStore.pushToast('無法取得網路介面，請重新整理後再試', 'danger');
    return false;
  }

  const payload: NetworkConfigUpdatePayload = {
    mode: 'MANUAL',
    interface_name: iface,
    ip_address: manualForm.ip_address.trim(),
    netmask: manualForm.netmask.trim(),
    gateway: manualForm.gateway.trim(),
    dns: buildDnsList()
  };

  let hasError = false;
  if (!payload.ip_address) {
    manualErrors.ip_address = '請輸入 IP 位址';
    hasError = true;
  }
  if (!payload.netmask) {
    manualErrors.netmask = '請輸入子網路遮罩';
    hasError = true;
  }
  if (!payload.gateway) {
    manualErrors.gateway = '請輸入預設路由';
    hasError = true;
  }
  if (!payload.dns?.length) {
    manualErrors.primaryDns = '請輸入至少一組 DNS';
    hasError = true;
  }

  if (hasError) {
    uiStore.pushToast('請檢查紅框欄位', 'info');
    return false;
  }

  isApplyingManual.value = true;
  try {
    const response = await updateNetworkConfig(payload);
    networkInfo.value = shapeNetworkInfo(response.network);
    syncManualForm();
    uiStore.pushToast(response.message || '已套用手動網路設定', 'success');
    return true;
  } catch (error) {
    console.error(error);
    let message = '更新網路設定失敗';
    if (error instanceof HttpError) {
      const payload = (error.payload ?? {}) as NetworkConfigUpdateResponse & {
        fields?: Record<string, string>;
      };
      applyFieldErrors(payload.fields);
      message = payload.message || message;
    } else if (error instanceof Error) {
      message = error.message;
    }
    uiStore.pushToast(message, 'danger');
    return false;
  } finally {
    isApplyingManual.value = false;
  }
};

const switchToDhcp = async (): Promise<boolean> => {
  if (!canConfigure.value || isSwitchingDhcp.value) return false;
  const iface = interfaceName.value.trim();
  if (!iface) {
    uiStore.pushToast('無法取得網路介面，請重新整理後再試', 'danger');
    return false;
  }
  isSwitchingDhcp.value = true;
  try {
    const response = await updateNetworkConfig({
      mode: 'DHCP',
      interface_name: iface
    });
    networkInfo.value = shapeNetworkInfo(response.network);
    syncManualForm();
    uiStore.pushToast(response.message || '已切換為 DHCP 模式', 'success');
    return true;
  } catch (error) {
    console.error(error);
    let message = '切換 DHCP 失敗';
    if (error instanceof HttpError) {
      const payload = (error.payload ?? {}) as NetworkConfigUpdateResponse;
      message = payload.message || message;
    } else if (error instanceof Error) {
      message = error.message;
    }
    uiStore.pushToast(message, 'danger');
    return false;
  } finally {
    isSwitchingDhcp.value = false;
  }
};

const submitNetworkConfig = async () => {
  if (!canConfigure.value) {
    return;
  }
  if (desiredMode.value === 'DHCP') {
    await switchToDhcp();
  } else {
    await applyManualConfig();
  }
};

onMounted(() => {
  loadPorts();
  loadNetwork();
  portPollingTimer.value = window.setInterval(loadPorts, 3000);
});

onBeforeUnmount(() => {
  if (portPollingTimer.value !== null) {
    window.clearInterval(portPollingTimer.value);
    portPollingTimer.value = null;
  }
});
</script>
