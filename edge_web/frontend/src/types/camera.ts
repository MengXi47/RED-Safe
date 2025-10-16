export interface Camera {
  ip: string;
  mac: string;
  name: string;
  is_bound?: boolean;
}

export interface CameraScanResponse {
  ok: boolean;
  results: Camera[];
  error?: string;
}

export interface CameraBindPayload {
  ip_address: string;
  mac_address: string;
  ipc_name: string;
  custom_name: string;
  ipc_account?: string;
  ipc_password?: string;
}
