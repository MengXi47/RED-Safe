import { http } from '@/lib/http';
import type {
  CameraBindPayload,
  CameraBindResponse,
  CameraPreviewOfferPayload,
  CameraPreviewOfferResponse,
  CameraPreviewProbePayload,
  CameraScanResponse,
  CameraUnbindResponse
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
    json: payload
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
    json: payload
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
