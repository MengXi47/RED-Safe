import { http } from '@/lib/http';
import type { BoundUserResponse, RemoveBoundUserResponse } from '@/types/user';

/**
 * 函式用途：取得目前已綁定裝置的使用者列表。
 * @returns Promise 解析為綁定使用者陣列。
 * @throws HttpError - 當網路或後端回傳錯誤時會拋出。
 */
export function fetchBoundUsers() {
  return http<BoundUserResponse>('/api/user/bound/');
}

/**
 * 函式用途：解除指定 Email 使用者與裝置的綁定關係。
 * @param email - 要解除綁定的使用者 Email。
 * @returns Promise 解析為操作狀態與訊息。
 * @throws HttpError - 當網路或後端回傳錯誤時會拋出。
 */
export function removeBoundUser(email: string) {
  return http<RemoveBoundUserResponse>('/api/user/remove/', {
    method: 'POST',
    json: { email }
  });
}
