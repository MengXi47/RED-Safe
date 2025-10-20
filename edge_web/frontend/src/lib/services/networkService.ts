import { http } from '@/lib/http';
import type { NetworkInterfaceInfo, PortStatusResponse } from '@/types/network';

/**
 * 函式用途：取得裝置目前的網路介面資訊。
 * @returns Promise 解析為 IP、網關與 DNS 設定。
 * @throws HttpError - 當網路或後端回傳錯誤時會拋出。
 */
export function fetchNetworkInfo() {
  return http<NetworkInterfaceInfo>('/network/ip/?format=json');
}

/**
 * 函式用途：查詢常用服務的連接埠狀態。
 * @returns Promise 解析為各連接埠是否運作與偵測到的號碼。
 * @throws HttpError - 當網路或後端回傳錯誤時會拋出。
 */
export function fetchPortStatus() {
  return http<PortStatusResponse>('/api/network/port/');
}
