import { http } from '@/lib/http';
import type { Camera, CameraBindPayload, CameraScanResponse } from '@/types/camera';

export function scanCameras() {
  return http<CameraScanResponse>('/api/cameras/scan');
}

export function bindCamera(payload: CameraBindPayload) {
  return http<{ ok: boolean; item: Camera; error?: string; code?: string }>('/api/cameras/bind', {
    method: 'POST',
    json: payload
  });
}

export function unbindCamera(macOrIp: { mac_address?: string; ip_address?: string }) {
  return http<{ ok: boolean; item?: Camera; error?: string }>('/api/cameras/unbind', {
    method: 'POST',
    json: macOrIp
  });
}

export function previewProbe(payload: { ip: string; account?: string; password?: string }) {
  return http<{ ok: boolean; error?: string; code?: string }>('/api/cameras/preview/probe', {
    method: 'POST',
    json: payload
  });
}

export function previewOffer(payload: Record<string, unknown>) {
  return http<{ ok: boolean; answer: { type: string; sdp: string }; session_id: string }>(
    '/api/cameras/preview/webrtc/offer',
    {
      method: 'POST',
      json: payload
    }
  );
}

export function previewHangup(payload: { session_id: string }) {
  return http<{ ok: boolean }>('/api/cameras/preview/webrtc/hangup', {
    method: 'POST',
    json: payload
  });
}
