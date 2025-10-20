import type { Camera } from './camera';
import type { DeviceInfoSnapshot } from './device';
import type { NetworkInterfaceInfo, PortStatusResponse } from './network';
import type { BoundUser } from './user';

/**
 * 檔案用途：描述伺服器注入於 window 物件的初始資料結構。
 * 與其他模組關聯：main.ts 在掛載前會寫入此資料，個別頁面透過 useInitialState 取得。
 */

export interface CamerasBootstrapState {
  searchResults?: Camera[];
  bound?: Camera[];
}

export interface EdgeInitialState extends Record<string, unknown> {
  cameras?: CamerasBootstrapState;
  users?: BoundUser[];
  network?: Partial<NetworkInterfaceInfo>;
  ports?: Partial<PortStatusResponse>;
  device?: DeviceInfoSnapshot;
  formErrors?: Record<string, string>;
}

declare global {
  interface Window {
    __EDGE_INITIAL_STATE__?: EdgeInitialState;
  }
}

export {};
