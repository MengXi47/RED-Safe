/**
 * 型別用途：描述系統日誌結構與 API 回傳格式，供前端統一使用。
 */

export interface SystemLogEntry {
  id: number;
  category: string;
  action: string;
  level: string;
  message: string;
  metadata: Record<string, unknown> | null;
  created_at: string;
}

export interface SystemLogListResponse {
  ok: boolean;
  items: SystemLogEntry[];
  total: number;
  limit: number;
  offset: number;
  max_rows?: number;
}

export interface SystemLogCreatePayload {
  action: string;
  message: string;
  level?: string;
  category?: string;
  metadata?: Record<string, unknown> | null;
}

export interface SystemLogCreateResponse {
  ok: boolean;
  item: SystemLogEntry;
  max_rows?: number;
}
