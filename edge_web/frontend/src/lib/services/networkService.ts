import { http } from '@/lib/http';
import type { NetworkInterfaceInfo, PortStatusResponse } from '@/types/network';

export function fetchNetworkInfo() {
  return http<NetworkInterfaceInfo>('/network/ip/?format=json');
}

export function fetchPortStatus() {
  return http<PortStatusResponse>('/api/network/port/');
}
