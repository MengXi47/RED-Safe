import { http } from '@/lib/http';
import type {
  SystemLogCreatePayload,
  SystemLogCreateResponse,
  SystemLogListResponse
} from '@/types/log';

/**
 * 模組用途：封裝系統日誌相關 API，統一處理查詢與新增。
 * 與其他模組關聯：LogsView 及未來需要寫入日誌的頁面會呼叫此處函式。
 */

export interface FetchSystemLogsParams {
  limit?: number;
  offset?: number;
  level?: string;
  category?: string;
}

export function fetchSystemLogs(params: FetchSystemLogsParams = {}) {
  const query = new URLSearchParams();
  if (typeof params.limit === 'number') {
    query.set('limit', String(params.limit));
  }
  if (typeof params.offset === 'number') {
    query.set('offset', String(params.offset));
  }
  if (params.level) {
    query.set('level', params.level);
  }
  if (params.category) {
    query.set('category', params.category);
  }
  const queryString = query.toString();
  const url = queryString ? `/api/system/logs?${queryString}` : '/api/system/logs';
  return http<SystemLogListResponse>(url);
}

export function createSystemLog(payload: SystemLogCreatePayload) {
  return http<SystemLogCreateResponse>('/api/system/logs', {
    method: 'POST',
    json: payload
  });
}
