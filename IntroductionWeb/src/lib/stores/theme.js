import { writable } from 'svelte/store';
import { browser } from '$app/environment';

function detectInitialTheme() {
  if (!browser) return 'light';
  const fromStorage = localStorage.getItem('theme');
  if (fromStorage === 'light' || fromStorage === 'dark') return fromStorage;
  const prefersDark = window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches;
  return prefersDark ? 'dark' : 'light';
}

export const theme = writable(detectInitialTheme());

if (browser) {
  /** @param {string} val */
  const apply = (val) => {
    document.documentElement.dataset.theme = val;
    localStorage.setItem('theme', val);
  };
  // apply immediately for first load
  apply(detectInitialTheme());
  theme.subscribe(apply);
}
