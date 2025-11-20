import { http } from '@/lib/http';
import type { MetricsResponse } from '@/types/metrics';

/**
 * 函式用途：取得儀表板所需的系統資源監控資料。
 * @returns Promise 解析為 CPU、記憶體等指標。
 * @throws HttpError - 當網路或後端回傳錯誤時會拋出。
 */
export function fetchMetrics() {
  return http<MetricsResponse>('/api/metrics/');
}
