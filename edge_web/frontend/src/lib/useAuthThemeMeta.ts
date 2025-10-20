import { onMounted } from 'vue';

/**
  * Hook 用途：針對登入/註冊頁設定瀏覽器狀態列顏色，維持品牌風格。
  */

const upsertMeta = (media: string, content: string) => {
  let meta = document.querySelector<HTMLMetaElement>(`meta[name="theme-color"][media="${media}"]`);
  if (!meta) {
    meta = document.createElement('meta');
    meta.name = 'theme-color';
    meta.media = media;
    document.head.appendChild(meta);
  }
  meta.content = content;
};

export const useAuthThemeMeta = () => {
  onMounted(() => {
    upsertMeta('(prefers-color-scheme: light)', '#f7f7f8');
    upsertMeta('(prefers-color-scheme: dark)', '#0f1115');
  });
};
