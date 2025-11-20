import { http } from '@/lib/http';
import type { DeviceInfo } from '@/types/device';

interface DeviceInfoResponse {
  device: DeviceInfo;
}

/**
 * 取得本地資料庫中儲存的裝置資訊。
 */
export function fetchDeviceInfo() {
  return http<DeviceInfoResponse>('/api/device/info/');
}

