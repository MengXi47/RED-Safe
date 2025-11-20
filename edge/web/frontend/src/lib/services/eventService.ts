import { http } from '@/lib/http';
import type { EdgeEventResponse } from '@/types/event';

export function fetchEvents(params: { start?: string; end?: string } = {}) {
  const search = new URLSearchParams();
  if (params.start) search.set('start', params.start);
  if (params.end) search.set('end', params.end);
  const qs = search.toString();
  const path = qs ? `/api/events/?${qs}` : '/api/events/';
  return http<EdgeEventResponse>(path);
}
