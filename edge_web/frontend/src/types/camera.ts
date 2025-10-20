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

export type CameraActionCode = 'AUTH_REQUIRED' | 'ALREADY_BOUND' | 'UNKNOWN_ERROR' | (string & {});

export interface CameraBindResponse {
  ok: boolean;
  item?: Camera;
  error?: string;
  code?: CameraActionCode;
}

export interface CameraUnbindResponse {
  ok: boolean;
  item?: Camera;
  error?: string;
  code?: string;
}

export interface CameraPreviewProbePayload {
  ip: string;
  account?: string;
  password?: string;
}

export interface CameraPreviewOfferPayload extends CameraPreviewProbePayload {
  offer: {
    type: RTCSdpType;
    sdp: string;
  };
}

export interface CameraPreviewOfferResponse {
  ok: boolean;
  answer: RTCSessionDescriptionInit;
  session_id: string;
  error?: string;
  code?: CameraActionCode;
}
