import { http } from '@/lib/http';
import type { MetricsResponse } from '@/types/metrics';

export function fetchMetrics() {
  return http<MetricsResponse>('/api/metrics/');
}
