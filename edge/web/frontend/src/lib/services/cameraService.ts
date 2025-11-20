import { http } from '@/lib/http';
import type {
  CameraBindPayload,
  CameraBindResponse,
  CameraPreviewOfferPayload,
  CameraPreviewOfferResponse,
  CameraPreviewProbePayload,
  CameraScanResponse,
  CameraBoundListResponse,
  CameraUnbindResponse,
  CameraPolicies,
  CameraPoliciesUpdatePayload
} from '@/types/camera';
/**
 * 函式用途：向後端發出攝影機掃描請求，取得附近裝置清單。
 * @returns Promise 解析為掃描結果與狀態。
 * @throws HttpError - 當網路或後端回傳錯誤時會拋出。
 */
export function scanCameras() {
  return http<CameraScanResponse>('/api/cameras/scan');
}

/**
 * 函式用途：將選取的攝影機綁定至系統。
 * @param payload - 綁定所需的攝影機網路與登入資訊。
 * @returns Promise 解析為綁定結果與攝影機資料。
 * @throws HttpError - 當網路或後端回傳錯誤時會拋出。
 */
export function bindCamera(payload: CameraBindPayload) {
  return http<CameraBindResponse>('/api/cameras/bind', {
    method: 'POST',
    json: payload
  });
}

/**
 * 函式用途：解除攝影機與系統的綁定關係。
 * @param macOrIp - 以 MAC 或 IP 主動解除綁定。
 * @returns Promise 解析為解除結果。
 * @throws HttpError - 當網路或後端回傳錯誤時會拋出。
 */
export function unbindCamera(macOrIp: { mac_address?: string; ip_address?: string }) {
  return http<CameraUnbindResponse>('/api/cameras/unbind', {
    method: 'POST',
    json: macOrIp
  });
}

/**
 * 函式用途：檢查攝影機是否需要帳密才能預覽串流。
 * @param payload - 包含攝影機 IP 與可選帳密。
 * @returns Promise 解析為探測結果與錯誤碼。
 * @throws HttpError - 當網路或後端回傳錯誤時會拋出。
 */
export function previewProbe(payload: CameraPreviewProbePayload) {
  return http<{ ok: boolean; error?: string; code?: string }>('/api/cameras/preview/probe', {
    method: 'POST',
    json: payload,
    // 與後端探測逾時(可調整)同步，避免前端先行中斷
    timeout: 35000
  });
}

/**
 * 函式用途：送出 WebRTC offer 以取得攝影機串流。
 * @param payload - 包含攝影機登入資訊與 WebRTC offer。
 * @returns Promise 解析為答覆資訊與會話編號。
 * @throws HttpError - 當網路或後端回傳錯誤時會拋出。
 */
export function previewOffer(payload: CameraPreviewOfferPayload) {
  return http<CameraPreviewOfferResponse>('/api/cameras/preview/webrtc/offer', {
    method: 'POST',
    json: payload,
    // WebRTC 建立過程需要與攝影機協商，延長逾時避免前端先中斷
    timeout: 45000
  });
}

/**
 * 函式用途：通知後端結束 WebRTC 預覽連線。
 * @param payload - 目前的預覽 session 編號。
 * @returns Promise 解析為操作成功與否。
 * @throws HttpError - 當網路或後端回傳錯誤時會拋出。
 */
export function previewHangup(payload: { session_id: string }) {
  return http<{ ok: boolean }>('/api/cameras/preview/webrtc/hangup', {
    method: 'POST',
    json: payload
  });
}

/**
 * 函式用途：向後端請求目前已綁定的攝影機清單。
 * @returns Promise 解析為已綁定的攝影機陣列與總筆數。
 */
export function fetchBoundCameras() {
  return http<CameraBoundListResponse>('/api/cameras/bound');
}

/**
 * 取得單一攝影機的偵測設定（跌倒、長時間靜止）。
 */
export function fetchCameraPolicies(ip_address: string) {
  const query = encodeURIComponent(ip_address);
  return http<CameraPolicies>(`/api/cameras/policies?ip_address=${query}`);
}

/**
 * 更新單一攝影機的偵測設定。
 */
export function updateCameraPolicies(payload: CameraPoliciesUpdatePayload) {
  return http<CameraPolicies>('/api/cameras/policies', {
    method: 'POST',
    json: payload
  });
}

/** 單獨更新跌倒偵測設定 */
export function updateFallPolicy(payload: { ip_address: string; enabled: boolean }) {
  return http<{ ok: boolean; fall_detection: { enabled: boolean } }>('/api/cameras/policies/fall', {
    method: 'POST',
    json: payload
  });
}

/** 單獨更新靜止/未活動設定 */
export function updateInactivityPolicy(payload: {
  ip_address: string;
  inactivity: {
    enabled: boolean;
    idle_minutes: number;
    quiet_start?: string | null;
    quiet_end?: string | null;
    quiet_enabled?: boolean;
  };
}) {
  return http<{ ok: boolean }>('/api/cameras/policies/inactivity', {
    method: 'POST',
    json: payload
  });
}

/** 單獨更新床區 ROI 設定 */
export function updateBedRoi(payload: { ip_address: string; bed_roi: { enabled: boolean; points: any[] } }) {
  return http<{ ok: boolean }>('/api/cameras/bed_roi', {
    method: 'POST',
    json: payload
  });
}

/** 取得床區 ROI 標註用的即時影像（data URL） */
export function fetchBedSnapshot(ip_address: string) {
  const query = encodeURIComponent(ip_address);
  return http<{ ok: boolean; data_url?: string; error?: string }>(`/api/cameras/bed_roi/snapshot?ip_address=${query}`, {
    retry: 1
  });
}
