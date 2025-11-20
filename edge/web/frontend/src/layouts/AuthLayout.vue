<template>
  <div
    class="relative min-h-screen min-h-dvh overflow-hidden text-ink"
    :style="{ background: 'var(--gradient-page)' }"
  >
    <div
      aria-hidden="true"
      class="pointer-events-none absolute inset-0 bg-[radial-gradient(circle_at_18%_18%,rgba(37,99,235,0.14),transparent_55%),radial-gradient(circle_at_82%_8%,rgba(37,99,235,0.12),transparent_50%)] opacity-75"
    />
    <main
      class="relative z-10 flex min-h-screen min-h-dvh items-center justify-center px-4 py-12 sm:px-6 lg:px-8"
    >
      <section
        class="w-full rounded-[28px] border border-border bg-surface-default p-8 shadow-elev-lg backdrop-blur-2xl transition-all duration-200 sm:p-10"
        :class="cardClass"
        :style="{ maxWidth: `${cardMaxWidth}px` }"
        :aria-labelledby="sectionLabelledby ?? undefined"
        :aria-describedby="sectionDescribedby ?? undefined"
      >
        <div class="flex flex-col items-center gap-6 text-center">
          <div v-if="hasLogo" class="flex justify-center">
            <slot name="logo" />
          </div>

          <header class="flex w-full flex-col gap-2">
            <slot name="title">
              <h1
                v-if="title"
                :id="titleId"
                class="text-balance text-2xl font-semibold tracking-wide text-ink sm:text-[1.75rem]"
              >
                {{ title }}
              </h1>
            </slot>
            <slot name="subtitle">
              <p
                v-if="subtitle"
                :id="subtitleId"
                class="text-pretty text-sm font-medium text-ink-muted sm:text-base"
              >
                {{ subtitle }}
              </p>
            </slot>
          </header>

          <div class="w-full">
            <slot />
          </div>

          <div v-if="hasFooter" class="w-full text-sm text-ink-muted">
            <slot name="footer" />
          </div>
        </div>
      </section>
    </main>
  </div>
</template>

<script setup lang="ts">
import { computed, useSlots, useId } from 'vue';

/**
  * 組件用途：建立登入/註冊頁專用的背景版型與卡片排版。
  * 輸入參數：title、subtitle、maxWidth、cardClass 控制版面。
  * 與其他模組關聯：LoginView、RegisterView 等驗證頁面沿用此版型。
  */

const props = withDefaults(
  defineProps<{
    title?: string;
    subtitle?: string;
    maxWidth?: number;
    cardClass?: string;
  }>(),
  {
    title: '',
    subtitle: '',
    maxWidth: 420,
    cardClass: ''
  }
);

const slots = useSlots();

// 判斷是否自訂 logo/footer slot，以調整呈現
const hasLogo = computed(() => Boolean(slots.logo));
const hasFooter = computed(() => Boolean(slots.footer));

const titleId = useId();
const subtitleId = useId();

// 依照 props 或 slot 決定 ARIA 關聯，保持無障礙資訊完整
const sectionLabelledby = computed(() => {
  if (slots.title) return null;
  return props.title ? titleId : null;
});

const sectionDescribedby = computed(() => {
  if (slots.subtitle) return null;
  return props.subtitle ? subtitleId : null;
});

const cardMaxWidth = computed(() => Math.max(320, props.maxWidth));
</script>
