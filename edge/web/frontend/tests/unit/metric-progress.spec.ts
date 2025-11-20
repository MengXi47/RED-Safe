import { describe, expect, it } from 'vitest';
import { mount } from '@vue/test-utils';
import MetricProgress from '@/components/visual/MetricProgress.vue';

describe('MetricProgress', () => {
  it('renders value and label', () => {
    const wrapper = mount(MetricProgress, {
      props: { value: 72, label: 'CPU 使用率' }
    });
    expect(wrapper.text()).toContain('72%');
    expect(wrapper.text()).toContain('CPU 使用率');
  });
});
