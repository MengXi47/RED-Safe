export interface EdgeEventItem {
  id: number;
  occurred_at: string;
  event_type: string;
  payload?: Record<string, unknown> | null;
}

export interface EdgeEventResponse {
  ok: boolean;
  events: EdgeEventItem[];
  count: number;
}
