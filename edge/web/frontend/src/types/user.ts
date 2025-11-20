export interface BoundUser {
  email: string;
  user_name: string;
  bind_at?: string;
  last_online?: string;
}

export interface BoundUserResponse {
  items: BoundUser[];
  error?: string;
}

export interface RemoveBoundUserResponse {
  status: string;
  message?: string;
}
