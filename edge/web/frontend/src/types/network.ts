export interface NetworkInterfaceInfo {
  interface_name: string;
  ip_address: string;
  netmask: string;
  gateway: string;
  dns: string[];
  mode: string;
  mode_raw: string;
}

export interface PortGroup {
  listening: boolean;
  ports: number[];
}

export interface PortStatusResponse {
  host: string;
  http: PortGroup;
  https: PortGroup;
  ssh: PortGroup;
}

export interface NetworkCapabilities {
  canConfigure: boolean;
}

export type NetworkConfigMode = 'MANUAL' | 'DHCP';

export interface NetworkConfigUpdatePayload {
  mode: NetworkConfigMode;
  interface_name: string;
  ip_address?: string;
  netmask?: string;
  gateway?: string;
  dns?: string[];
}

export interface NetworkConfigUpdateResponse {
  success: boolean;
  message: string;
  network: NetworkInterfaceInfo;
  error?: string;
  fields?: Record<string, string>;
}
