export type ThemeMode = 'light' | 'dark';
export type ThemePreference = ThemeMode | 'system';

const THEME_STORAGE_KEY = 'edge.theme.preference';

export const getStoredPreference = (): ThemePreference | null => {
  if (typeof window === 'undefined') return null;
  const saved = window.localStorage.getItem(THEME_STORAGE_KEY);
  return saved === 'dark' || saved === 'light' || saved === 'system' ? saved : null;
};

export const persistPreference = (preference: ThemePreference) => {
  if (typeof window === 'undefined') return;
  window.localStorage.setItem(THEME_STORAGE_KEY, preference);
};

export const getSystemTheme = (): ThemeMode => {
  if (typeof window !== 'undefined' && typeof window.matchMedia === 'function') {
    return window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
  }
  return 'light';
};

export const resolveEffectiveTheme = (preference: ThemePreference): ThemeMode => {
  return preference === 'system' ? getSystemTheme() : preference;
};

export const applyThemePreference = (preference: ThemePreference) => {
  if (typeof document === 'undefined') return;
  if (preference === 'system') {
    document.documentElement.removeAttribute('data-theme');
  } else {
    document.documentElement.setAttribute('data-theme', preference);
  }
};

export const resolveThemePreference = (): ThemePreference => {
  const stored = getStoredPreference();
  if (stored) return stored;
  return 'system';
};

export const bootstrapTheme = (): ThemePreference => {
  const preference = resolveThemePreference();
  applyThemePreference(preference);
  return preference;
};

export const subscribeSystemTheme = (callback: (mode: ThemeMode) => void) => {
  if (typeof window === 'undefined' || typeof window.matchMedia !== 'function') {
    return () => {};
  }

  const media = window.matchMedia('(prefers-color-scheme: dark)');
  const handler = (event: MediaQueryListEvent) => {
    callback(event.matches ? 'dark' : 'light');
  };

  if (typeof media.addEventListener === 'function') {
    media.addEventListener('change', handler);
    return () => media.removeEventListener('change', handler);
  }

  if (typeof media.addListener === 'function') {
    media.addListener(handler);
    return () => media.removeListener(handler);
  }

  return () => {};
};
