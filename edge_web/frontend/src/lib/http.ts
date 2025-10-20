import axios, {
  AxiosError,
  type AxiosInstance,
  type AxiosRequestConfig,
  type AxiosResponse
} from 'axios';

/**
 * 模組用途：建立具備 CSRF、自動重試與統一錯誤格式的 Axios 客戶端。
 * 與其他模組關聯：service 層統一透過此函式發送 API 請求。
 */

export interface HttpClientOptions extends AxiosRequestConfig {
  /**
   * 方便以 json 傳入 body，會自動轉成 data 與設定 Content-Type。
   */
  json?: unknown;
  /**
   * 當遇到網路錯誤時的重試次數，預設不重試。
   */
  retry?: number;
}

export class HttpError<T = unknown> extends Error {
  /**
   * @param message - 顯示給呼叫端的錯誤訊息。
   * @param status - HTTP 狀態碼，若為網路錯誤則為 undefined。
   * @param code - 後端回傳的錯誤代碼。
   * @param payload - 後端原始回傳資料，便於客製處理。
   * @param isNetworkError - 是否屬於網路層錯誤（如逾時、斷線）。
   */
  constructor(
    message: string,
    public status?: number,
    public code?: string,
    public payload?: T,
    public isNetworkError: boolean = false
  ) {
    super(message);
    this.name = 'HttpError';
  }
}

const csrfToken = (() => {
  if (typeof document === 'undefined') return '';
  const match = document.cookie.match(/csrftoken=([^;]+)/);
  return match ? match[1] : '';
})();

const client: AxiosInstance = axios.create({
  headers: {
    'X-Requested-With': 'XMLHttpRequest'
  },
  withCredentials: true,
  timeout: 15000
});

client.interceptors.request.use((config) => {
  if (!config.headers) {
    config.headers = {};
  }
  // 若呼叫端尚未設定 CSRF Token，則統一補上
  if (!config.headers['X-CSRFToken'] && csrfToken) {
    config.headers['X-CSRFToken'] = csrfToken;
  }
  return config;
});

client.interceptors.response.use(
  (response: AxiosResponse) => response,
  (error: AxiosError) => {
    return Promise.reject(transformAxiosError(error));
  }
);

const sleep = (ms: number) => new Promise((resolve) => setTimeout(resolve, ms));

const transformAxiosError = (error: AxiosError): HttpError => {
  if (!error.isAxiosError) {
    return new HttpError('發生未知的網路錯誤', undefined, undefined, undefined, true);
  }

  if (!error.response) {
    const message = error.message || '無法連線至伺服器';
    return new HttpError(message, undefined, error.code, undefined, true);
  }

  const { status, data } = error.response;
  const payload = data as { error?: string; message?: string; code?: string };
  const message = payload?.error || payload?.message || error.message || '伺服器回傳未知錯誤';
  return new HttpError(message, status, payload?.code, payload, false);
};

/**
 * 封裝 axios，維持舊有呼叫介面並提供重試機制。
 * @param url - API 路徑。
 * @param options - 客製請求設定。
 * @returns Promise 解析為後端回傳資料。
 */
export async function http<T>(url: string, options: HttpClientOptions = {}): Promise<T> {
  const { json, retry = 0, headers, ...rest } = options;

  const config: AxiosRequestConfig = {
    url,
    method: rest.method ?? 'GET',
    ...rest,
    headers: {
      Accept: 'application/json',
      ...headers
    }
  };

  if (json !== undefined) {
    config.data = json;
    config.headers = {
      ...config.headers,
      'Content-Type': 'application/json'
    };
  }

  let attempt = 0;
  while (true) {
    try {
      const response = await client.request<T>(config);
      return response.data;
    } catch (error) {
      const normalized = error instanceof HttpError ? error : transformAxiosError(error as AxiosError);
      if (normalized.isNetworkError && attempt < retry) {
        attempt += 1;
        await sleep(300 * attempt);
        continue;
      }
      throw normalized;
    }
  }
}
