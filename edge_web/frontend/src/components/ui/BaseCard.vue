<template>
  <section class="rounded-3xl border border-slate-200 bg-white p-6 shadow-soft">
    <header v-if="title || $slots.header" class="mb-4 flex items-start justify-between gap-4">
      <div>
        <h2 v-if="title" class="text-lg font-semibold text-ink">{{ title }}</h2>
        <p v-if="description" class="text-sm text-ink-muted">{{ description }}</p>
      </div>
      <slot name="header" />
    </header>
    <div class="space-y-4">
      <slot />
    </div>
    <footer v-if="$slots.footer" class="mt-6">
      <slot name="footer" />
    </footer>
  </section>
</template>

<script setup lang="ts">
defineProps<{
  title?: string;
  description?: string;
}>();
</script>
