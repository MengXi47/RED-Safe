<template>
  <div class="metric-sparkline">
    <svg
      v-if="hasData"
      :viewBox="`0 0 ${VIEWBOX_WIDTH} ${VIEWBOX_HEIGHT}`"
      preserveAspectRatio="none"
      class="h-16 w-full"
      aria-hidden="true"
    >
      <path
        v-if="areaPath"
        :d="areaPath"
        fill="currentColor"
        fill-opacity="0.18"
      />
      <path
        :d="linePath"
        stroke="currentColor"
        stroke-width="2.4"
        fill="none"
        stroke-linecap="round"
        stroke-linejoin="round"
      />
    </svg>
    <div v-else class="h-16 w-full rounded-xl border border-dashed border-border/60"></div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue';

const VIEWBOX_WIDTH = 100;
const VIEWBOX_HEIGHT = 40;

const props = defineProps<{
  points: number[];
}>();

const sanitizedPoints = computed(() =>
  props.points.filter((value) => Number.isFinite(value))
);

const hasData = computed(() => sanitizedPoints.value.length > 0);

const coords = computed(() => {
  const pts = sanitizedPoints.value;
  if (!pts.length) return [];
  const min = Math.min(...pts);
  const max = Math.max(...pts);
  const range = max - min || 1;
  const lastIndex = Math.max(pts.length - 1, 1);
  return pts.map((value, index) => {
    const x = (index / lastIndex) * VIEWBOX_WIDTH;
    const y = VIEWBOX_HEIGHT - ((value - min) / range) * VIEWBOX_HEIGHT;
    return { x, y };
  });
});

const linePath = computed(() => {
  const points = coords.value;
  if (!points.length) return '';
  return points
    .map((point, index) => `${index === 0 ? 'M' : 'L'}${point.x} ${point.y}`)
    .join(' ');
});

const areaPath = computed(() => {
  const points = coords.value;
  if (points.length < 2) return '';
  const first = points[0];
  const last = points[points.length - 1];
  return `${linePath.value} L${last.x} ${VIEWBOX_HEIGHT} L${first.x} ${VIEWBOX_HEIGHT} Z`;
});
</script>
