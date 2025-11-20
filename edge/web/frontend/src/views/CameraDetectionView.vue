<template>
  <div class="space-y-8">
    <header class="page-hero">
      <div class="flex flex-col gap-2">
        <h2 class="text-2xl font-semibold text-ink">偵測功能</h2>
      </div>

      <div class="mt-4 grid gap-3 rounded-xl border border-border bg-surface-subtle p-4 md:grid-cols-3 md:items-end">
        <div class="md:col-span-1">
          <label class="flex flex-col gap-1 text-sm text-ink">
            選擇攝影機
            <select
              v-model="selectedIp"
              class="mt-1 w-full rounded-lg border border-border px-3 py-2 text-sm text-ink outline-none focus:border-brand-500 focus:ring-2 focus:ring-brand-200"
            >
              <option v-if="cameras.length === 0" value="">尚無已綁定攝影機</option>
              <option v-for="cam in cameras" :key="cam.ip" :value="cam.ip">
                {{ cam.name || cam.ip }} ({{ cam.ip }})
              </option>
            </select>
          </label>
        </div>
        <div>
          <p class="text-xs text-ink-muted">攝影機 IP</p>
          <p class="text-lg font-semibold text-ink">{{ selectedCamera?.ip ?? '未選擇' }}</p>
        </div>
        <div>
          <p class="text-xs text-ink-muted">名稱</p>
          <p class="text-lg font-semibold text-ink">{{ selectedName }}</p>
        </div>
      </div>
    </header>

    <div v-if="selectedIp" class="flex flex-wrap gap-2">
      <button
        v-for="tab in tabs"
        :key="tab.key"
        type="button"
        class="rounded-full border px-4 py-2 text-sm font-semibold transition"
        :class="activeTab === tab.key ? 'border-brand-500 bg-brand-500/10 text-brand-700' : 'border-border text-ink-muted hover:text-ink'"
        @click="activeTab = tab.key"
      >
        {{ tab.label }}
      </button>
    </div>

    <BaseCard v-if="activeTab === 'fall' && selectedIp">
      <div class="flex flex-col gap-4">
        <header class="flex flex-col gap-1">
          <h3 class="text-lg font-semibold text-ink">跌倒偵測</h3>
          <p class="text-sm text-ink-muted">控制是否在邊緣裝置上啟用跌倒偵測。預設關閉，可視情況開啟。</p>
        </header>

        <div class="flex items-center justify-between rounded-xl border border-border bg-surface-subtle p-4">
          <div class="space-y-1">
            <p class="text-sm font-semibold text-ink">功能開關</p>
            <p class="text-xs text-ink-muted">開啟後對應攝影機會執行跌倒推論並回報事件。</p>
          </div>
          <label class="toggle">
            <input v-model="fallEnabled" type="checkbox" />
            <span class="toggle__track" aria-hidden="true">
              <span class="toggle__thumb" />
            </span>
            <span class="toggle__label">{{ fallEnabled ? '已開啟' : '已關閉' }}</span>
          </label>
        </div>

        <div class="flex justify-end">
          <BaseButton :loading="savingFall || loadingPolicies" @click="saveFallPolicies">儲存設定</BaseButton>
        </div>
      </div>
    </BaseCard>

    <BaseCard v-else-if="activeTab === 'inactivity' && selectedIp">
      <div class="flex flex-col gap-4">
        <header class="flex flex-col gap-1">
          <h3 class="text-lg font-semibold text-ink">長時間靜止/未活動</h3>
          <p class="text-sm text-ink-muted">
            控制是否偵測長時間靜止；可設定持續分鐘數與每日免偵測時段（例如 21:00 至 06:00）。
          </p>
        </header>

        <div class="grid gap-3 md:grid-cols-2">
          <div class="rounded-xl border border-border bg-surface-subtle p-4">
            <div class="flex items-center justify-between">
              <div class="space-y-1">
                <p class="text-sm font-semibold text-ink">功能開關</p>
                <p class="text-xs text-ink-muted">開啟後將依據門檻判定長時間未活動事件。</p>
              </div>
              <label class="toggle">
                <input v-model="inactivityEnabled" type="checkbox" />
                <span class="toggle__track" aria-hidden="true">
                  <span class="toggle__thumb" />
                </span>
                <span class="toggle__label">{{ inactivityEnabled ? '已開啟' : '已關閉' }}</span>
              </label>
            </div>
          </div>

          <div class="rounded-xl border border-border bg-surface-subtle p-4">
            <label class="flex flex-col gap-1 text-sm font-semibold text-ink">
              持續靜止門檻（分鐘）
              <input
                v-model.number="idleMinutes"
                type="number"
                min="1"
                class="mt-1 w-full rounded-lg border border-border px-3 py-2 text-sm text-ink outline-none focus:border-brand-500 focus:ring-2 focus:ring-brand-200"
              />
              <span class="text-xs font-normal text-ink-muted">超過此分鐘數未偵測到動作將觸發事件。</span>
            </label>
          </div>
        </div>

        <div class="rounded-xl border border-border bg-surface-subtle p-4">
          <div class="flex flex-col gap-3">
            <p class="text-sm font-semibold text-ink">每日免偵測時段</p>
            <label class="toggle">
              <input v-model="quietEnabled" type="checkbox" />
              <span class="toggle__track" aria-hidden="true">
                <span class="toggle__thumb" />
              </span>
              <span class="toggle__label">{{ quietEnabled ? '免偵測時段已開啟' : '免偵測時段關閉' }}</span>
            </label>
            <div class="grid gap-3 md:grid-cols-2">
              <label class="flex flex-col gap-1 text-sm text-ink">
                開始時間
                <input
                  v-model="quietStart"
                  type="time"
                  step="60"
                  :disabled="!quietEnabled"
                  class="mt-1 w-full rounded-lg border border-border px-3 py-2 text-sm text-ink outline-none focus:border-brand-500 focus:ring-2 focus:ring-brand-200"
                />
              </label>
              <label class="flex flex-col gap-1 text-sm text-ink">
                結束時間
                <input
                  v-model="quietEnd"
                  type="time"
                  step="60"
                  :disabled="!quietEnabled"
                  class="mt-1 w-full rounded-lg border border-border px-3 py-2 text-sm text-ink outline-none focus:border-brand-500 focus:ring-2 focus:ring-brand-200"
                />
              </label>
            </div>
            <p class="text-xs text-ink-muted">
              若開始時間晚於結束時間，代表跨越午夜（例如 21:00 至隔日 06:00）。
            </p>
          </div>
        </div>

        <div class="flex justify-end">
          <BaseButton :loading="savingInactivity || loadingPolicies" @click="saveInactivityPolicies">儲存設定</BaseButton>
        </div>
      </div>
    </BaseCard>

    <BaseCard v-else-if="activeTab === 'bed' && selectedIp">
      <div class="flex flex-col gap-4">
        <header class="flex flex-col gap-1">
          <h3 class="text-lg font-semibold text-ink">夜間離床</h3>
          <p class="text-sm text-ink-muted">設定床區範圍以判斷夜間離床事件。</p>
        </header>

        <div class="rounded-xl border border-border bg-surface-subtle p-4 space-y-3">
          <div class="flex items-center justify-between">
            <div class="space-y-1">
              <p class="text-sm font-semibold text-ink">功能開關</p>
              <p class="text-xs text-ink-muted">開啟後依據床區判斷夜間離床。</p>
            </div>
            <label class="toggle">
              <input v-model="bedRoiEnabled" type="checkbox" />
              <span class="toggle__track" aria-hidden="true">
                <span class="toggle__thumb" />
              </span>
              <span class="toggle__label">{{ bedRoiEnabled ? '已開啟' : '已關閉' }}</span>
            </label>
          </div>
          <label class="flex flex-col gap-1 text-sm text-ink">
            床區座標（點擊下方影像標記，最多四點）
          </label>
          <div class="grid gap-3 md:grid-cols-2">
            <label class="flex flex-col gap-1 text-sm text-ink">
              開始時間
              <input
                v-model="bedQuietStart"
                type="time"
                step="60"
                :disabled="!bedRoiEnabled"
                class="mt-1 w-full rounded-lg border border-border px-3 py-2 text-sm text-ink outline-none focus:border-brand-500 focus:ring-2 focus:ring-brand-200"
              />
            </label>
            <label class="flex flex-col gap-1 text-sm text-ink">
              結束時間
              <input
                v-model="bedQuietEnd"
                type="time"
                step="60"
                :disabled="!bedRoiEnabled"
                class="mt-1 w-full rounded-lg border border-border px-3 py-2 text-sm text-ink outline-none focus:border-brand-500 focus:ring-2 focus:ring-brand-200"
              />
            </label>
          </div>
          <div class="space-y-2">
            <div class="flex items-center gap-2">
              <BaseButton size="sm" variant="ghost" :loading="loadingPolicies" @click="fetchSnapshot">擷取即時影像</BaseButton>
              <BaseButton size="sm" variant="ghost" :disabled="!bedPointList.length" @click="clearBedPoints">清除已標記點</BaseButton>
            </div>
            <div v-if="snapshotDataUrl" class="relative inline-block overflow-hidden rounded-xl border border-border bg-surface-subtle">
              <img :src="snapshotDataUrl" alt="ROI snapshot" class="max-h-96 object-contain" />
              <div class="absolute inset-0" :class="{ 'cursor-crosshair': bedRoiEnabled }" @click="addPointOnSnapshot">
                <template v-for="(pt, idx) in bedPointList" :key="`pt-${idx}`">
                  <span
                    class="absolute -translate-x-1/2 -translate-y-1/2 rounded-full bg-brand-500 text-[10px] font-semibold text-white shadow"
                    :style="{ left: `${pt.x * 100}%`, top: `${pt.y * 100}%`, padding: '4px' }"
                  >
                    {{ idx + 1 }}
                  </span>
                </template>
              </div>
            </div>
          </div>
        </div>

        <div class="flex justify-end">
          <BaseButton :loading="savingBed || loadingPolicies" @click="saveBedPolicies">儲存床區設定</BaseButton>
        </div>
      </div>
    </BaseCard>

  </div>
</template>

<script setup lang="ts">
import { computed, onMounted, ref, watch } from 'vue';
import BaseButton from '@/components/ui/BaseButton.vue';
import BaseCard from '@/components/ui/BaseCard.vue';
import { useUiStore } from '@/store/ui';
import type { Camera } from '@/types/camera';
import {
  fetchBedSnapshot,
  fetchBoundCameras,
  fetchCameraPolicies,
  updateBedRoi,
  updateFallPolicy,
  updateInactivityPolicy
} from '@/lib/services/cameraService';

type TabKey = 'fall' | 'inactivity';

const tabs: { key: TabKey; label: string }[] = [
  { key: 'fall', label: '跌倒偵測' },
  { key: 'inactivity', label: '長時間靜止/未活動' },
  { key: 'bed', label: '夜間離床' }
];

const activeTab = ref<TabKey>('fall');
const cameras = ref<Camera[]>([]);
const selectedIp = ref('');
const selectedCamera = ref<Camera | null>(null);
const fallEnabled = ref(false);
const inactivityEnabled = ref(false);
const idleMinutes = ref(5);
const quietStart = ref('');
const quietEnd = ref('');
const quietEnabled = ref(false);
const bedRoiEnabled = ref(false);
const bedPointList = ref<Array<{ x: number; y: number }>>([]);
const snapshotDataUrl = ref('');
const bedQuietStart = ref('');
const bedQuietEnd = ref('');
const loadingCameras = ref(false);
const loadingPolicies = ref(false);
const savingFall = ref(false);
const savingInactivity = ref(false);
const savingBed = ref(false);

const uiStore = useUiStore();

const resetPolicies = () => {
  fallEnabled.value = false;
  inactivityEnabled.value = true;
  idleMinutes.value = 5;
  quietStart.value = '';
  quietEnd.value = '';
  quietEnabled.value = false;
  bedRoiEnabled.value = false;
  bedPointList.value = [];
  snapshotDataUrl.value = '';
  bedQuietStart.value = '';
  bedQuietEnd.value = '';
};

const ensureQuietFilled = () => {
  if (quietEnabled.value && (!quietStart.value || !quietEnd.value)) {
    uiStore.pushToast('請設定免偵測開始與結束時間', 'danger');
    return false;
  }
  return true;
};

const syncSelectedCamera = () => {
  selectedCamera.value = cameras.value.find((cam) => cam.ip === selectedIp.value) ?? null;
};

const loadPolicies = async () => {
  if (!selectedIp.value) {
    resetPolicies();
    selectedCamera.value = null;
    return;
  }
  loadingPolicies.value = true;
  try {
    const resp = await fetchCameraPolicies(selectedIp.value);
    fallEnabled.value = !!resp.fall_detection?.enabled;
    inactivityEnabled.value = !!resp.inactivity?.enabled;
    idleMinutes.value = Number(resp.inactivity?.idle_minutes ?? 5);
    quietStart.value = resp.inactivity?.quiet_start ?? '';
    quietEnd.value = resp.inactivity?.quiet_end ?? '';
    quietEnabled.value = !!resp.inactivity?.quiet_enabled;
    bedRoiEnabled.value = !!resp.bed_roi?.enabled;
    const parsed = resp.bed_roi?.points && Array.isArray(resp.bed_roi.points) ? resp.bed_roi.points : [];
    bedPointList.value = parsed;
    bedQuietStart.value = resp.bed_roi?.quiet_start ?? '';
    bedQuietEnd.value = resp.bed_roi?.quiet_end ?? '';
    if (activeTab.value === 'bed') {
      void fetchSnapshot(true, parsed);
    }
  } catch (error) {
    console.error(error);
    uiStore.pushToast('無法載入偵測設定', 'danger');
    resetPolicies();
  } finally {
    loadingPolicies.value = false;
  }
};

const loadCameras = async () => {
  loadingCameras.value = true;
  try {
    const response = await fetchBoundCameras();
    cameras.value = response.items ?? [];
    if (!selectedIp.value && cameras.value.length > 0) {
      selectedIp.value = cameras.value[0].ip;
    } else if (selectedIp.value && !cameras.value.find((cam) => cam.ip === selectedIp.value)) {
      selectedIp.value = cameras.value[0]?.ip ?? '';
    }
    syncSelectedCamera();
    if (activeTab.value === 'bed') {
      void fetchSnapshot(true, bedPointList.value);
    }
  } catch (error) {
    console.error(error);
    uiStore.pushToast('無法載入攝影機列表', 'danger');
  } finally {
    loadingCameras.value = false;
  }
};

const saveFallPolicies = async () => {
  if (!selectedIp.value) {
    uiStore.pushToast('請先選擇攝影機', 'danger');
    return;
  }
  if (!ensureQuietFilled()) return;
  savingFall.value = true;
  try {
    await updateFallPolicy({ ip_address: selectedIp.value, enabled: fallEnabled.value });
    uiStore.pushToast('已更新跌倒偵測設定', 'success');
  } catch (error) {
    console.error(error);
    uiStore.pushToast('更新失敗，請稍後再試', 'danger');
  } finally {
    savingFall.value = false;
  }
};

const saveInactivityPolicies = async () => {
  if (!selectedIp.value) {
    uiStore.pushToast('請先選擇攝影機', 'danger');
    return;
  }
  if (!ensureQuietFilled()) return;
  savingInactivity.value = true;
  try {
    await updateInactivityPolicy({
      ip_address: selectedIp.value,
      inactivity: {
        enabled: inactivityEnabled.value,
        idle_minutes: idleMinutes.value,
        quiet_start: quietStart.value || null,
        quiet_end: quietEnd.value || null,
        quiet_enabled: quietEnabled.value
      }
    });
    uiStore.pushToast('已更新靜止偵測設定', 'success');
  } catch (error) {
    console.error(error);
    uiStore.pushToast('更新失敗，請稍後再試', 'danger');
  } finally {
    savingInactivity.value = false;
  }
};

const saveBedPolicies = async () => {
  if (!selectedIp.value) {
    uiStore.pushToast('請先選擇攝影機', 'danger');
    return;
  }
  if (bedRoiEnabled.value && bedPointList.value.length !== 4) {
    uiStore.pushToast('請在影像上標記四個床位頂點', 'danger');
    return;
  }
  if (bedRoiEnabled.value && (!bedQuietStart.value || !bedQuietEnd.value)) {
    uiStore.pushToast('請設定夜間離床的每日偵測時段開始與結束時間', 'danger');
    return;
  }
  savingBed.value = true;
  try {
    await updateBedRoi({
      ip_address: selectedIp.value,
      bed_roi: {
        enabled: bedRoiEnabled.value,
        points: bedPointList.value ?? [],
        quiet_start: bedQuietStart.value || null,
        quiet_end: bedQuietEnd.value || null
      }
    });
    uiStore.pushToast('已更新夜間離床設定', 'success');
  } catch (error) {
    console.error(error);
    uiStore.pushToast('更新失敗，請稍後再試', 'danger');
  } finally {
    savingBed.value = false;
  }
};

onMounted(() => {
  void loadCameras();
});

watch(
  () => selectedIp.value,
  async () => {
    syncSelectedCamera();
    await loadPolicies();
  }
);

watch(
  () => activeTab.value,
  (tab) => {
    if (tab === 'bed') {
      void fetchSnapshot(true);
    }
  }
);

const selectedName = computed(() => selectedCamera.value?.name ?? '未選擇');

const addPointOnSnapshot = (event: MouseEvent) => {
  if (!snapshotDataUrl.value || !bedRoiEnabled.value) return;
  const target = event.currentTarget as HTMLElement | null;
  if (!target) return;
  const rect = target.getBoundingClientRect();
  const x = (event.clientX - rect.left) / rect.width;
  const y = (event.clientY - rect.top) / rect.height;
  const clamped = { x: Number(Math.max(0, Math.min(1, x)).toFixed(4)), y: Number(Math.max(0, Math.min(1, y)).toFixed(4)) };
  if (bedPointList.value.length >= 4) return;
  bedPointList.value = [...bedPointList.value, clamped];
};

const clearBedPoints = () => {
  bedPointList.value = [];
};

const fetchSnapshot = async (silent = false) => {
  if (!selectedIp.value) {
    if (!silent) uiStore.pushToast('請先選擇攝影機', 'danger');
    return;
  }
  try {
    const resp = await fetchBedSnapshot(selectedIp.value);
    if (!resp.ok || !resp.data_url) {
      if (!silent) uiStore.pushToast(resp.error || '擷取影像失敗', 'danger');
      return;
    }
    snapshotDataUrl.value = resp.data_url;
    if (!bedPointList.value.length) {
      // 如果後端已有既存座標不清除，讓標記顯示在圖片上
      bedPointList.value = bedPointList.value;
    }
  } catch (error) {
    console.error(error);
    if (!silent) uiStore.pushToast('擷取影像失敗', 'danger');
  }
};
</script>

<style scoped>
.toggle {
  display: inline-flex;
  align-items: center;
  gap: 0.6rem;
  cursor: pointer;
  font-weight: 600;
  color: var(--ink);
}

.toggle input {
  position: absolute;
  opacity: 0;
  pointer-events: none;
}

.toggle__track {
  position: relative;
  width: 48px;
  height: 26px;
  border-radius: 999px;
  background: color-mix(in srgb, var(--surface-subtle) 80%, transparent);
  border: 1px solid color-mix(in srgb, var(--color-border) 70%, transparent);
  box-shadow: inset 0 1px 2px rgba(15, 23, 42, 0.06);
  transition: background-color 160ms ease, border-color 160ms ease;
}

.toggle__thumb {
  position: absolute;
  top: 2px;
  left: 2px;
  width: 22px;
  height: 22px;
  border-radius: 50%;
  background: white;
  box-shadow: 0 2px 6px rgba(15, 23, 42, 0.16);
  transition: transform 180ms cubic-bezier(0.22, 1, 0.36, 1);
}

.toggle input:checked + .toggle__track {
  background: var(--color-accent-500, #22c55e);
  border-color: color-mix(in srgb, var(--color-accent-500, #22c55e) 80%, transparent);
}

.toggle input:checked + .toggle__track .toggle__thumb {
  transform: translateX(22px);
}

.toggle__label {
  font-size: 0.9rem;
  color: var(--ink);
}

</style>
