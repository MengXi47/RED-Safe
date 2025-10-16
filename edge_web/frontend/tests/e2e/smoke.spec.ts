import { test, expect } from '@playwright/test';

test('dashboard renders', async ({ page }) => {
  await page.goto('/');
  await expect(page.getByText('系統效能監控')).toBeVisible();
});
