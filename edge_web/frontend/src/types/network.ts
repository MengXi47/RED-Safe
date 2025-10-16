export interface NetworkInterfaceInfo {
  ip_address: string;
  netmask: string;
  gateway: string;
  dns: string[];
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
