export interface HttpClientOptions extends RequestInit {
  json?: unknown;
}

export class HttpError extends Error {
  constructor(
    message: string,
    public status: number,
    public payload?: unknown
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

export async function http<T>(input: string, options: HttpClientOptions = {}): Promise<T> {
  const headers = new Headers(options.headers);
  if (options.json !== undefined) {
    headers.set('Content-Type', 'application/json');
  }
  if (!headers.has('X-CSRFToken') && csrfToken) {
    headers.set('X-CSRFToken', csrfToken);
  }

  const response = await fetch(input, {
    ...options,
    headers,
    body: options.json !== undefined ? JSON.stringify(options.json) : options.body
  });

  const text = await response.text();
  const data = text ? JSON.parse(text) : undefined;

  if (!response.ok) {
    throw new HttpError(data?.error || response.statusText, response.status, data);
  }

  return data as T;
}
