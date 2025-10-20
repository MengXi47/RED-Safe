import type { EdgeInitialState } from '@/types/bootstrap';

/**
 * Hook 用途：安全地讀取伺服器注入的初始資料並回傳預設值。
 * 與其他模組關聯：各頁面可藉此取得 SSR 注入的資料並避免 any 型別。
 *
 * @param selector - 針對初始資料取值的函式，若資料不存在可回傳 undefined。
 * @param fallbackFactory - 建立預設值的函式，避免共用同一參考導致狀態污染。
 */
export const useInitialState = <T>(
  selector: (state: EdgeInitialState) => T | undefined,
  fallbackFactory: () => T
): T => {
  if (typeof window === 'undefined') {
    return fallbackFactory();
  }
  const state = window.__EDGE_INITIAL_STATE__;
  if (!state) {
    return fallbackFactory();
  }
  const selected = selector(state);
  return selected !== undefined ? selected : fallbackFactory();
};
