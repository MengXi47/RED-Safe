/**
 * 檔案用途：定義裝置資訊的型別結構，供頁面與服務層共用。
 * 與其他模組關聯：DeviceInfoView、相關 API 回應均需符合此型別。
 */

export interface DeviceInfo {
  serial: string;
  version: string;
  status: number;
  masked_password: string;
  password: string;
  qrcode?: string;
}

export type DeviceInfoSnapshot = Partial<DeviceInfo>;
