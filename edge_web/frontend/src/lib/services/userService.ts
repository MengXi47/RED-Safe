import { http } from '@/lib/http';
import type { BoundUserResponse } from '@/types/user';

export function fetchBoundUsers() {
  return http<BoundUserResponse>('/api/user/bound/');
}

export function removeBoundUser(email: string) {
  return http<{ status: string; message?: string }>('/api/user/remove/', {
    method: 'POST',
    json: { email }
  });
}
