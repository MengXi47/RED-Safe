import { expect, test } from '@playwright/test';

test.describe('auth routes', () => {
  test('login form interactions and CTA navigation', async ({ page }) => {
    await page.goto('/login');

    await expect(page.getByRole('heading', { name: '輸入密碼以登入系統' })).toBeVisible();

    const passwordField = page.getByLabel('密碼');
    await passwordField.fill('Secret1');

    const toggleButton = page.getByRole('button', { name: '顯示密碼' });
    await toggleButton.click();
    await expect(passwordField).toHaveAttribute('type', 'text');
    await expect(toggleButton).toHaveAttribute('aria-label', '隱藏密碼');

    const submitButton = page.getByRole('button', { name: '登入' });
    await submitButton.click();
    await expect(submitButton).toBeDisabled();
    await expect(submitButton).toBeEnabled({ timeout: 2000 });

    const registerLink = page.getByRole('link', { name: '去註冊' });
    await expect(registerLink).toHaveAttribute('href', '/register');
    await registerLink.click();
    await expect(page).toHaveURL(/\/register$/);
  });

  test('register layout matches login and supports submission flow', async ({ page }) => {
    await page.goto('/login');
    const loginCardClass = await page.locator('main section').first().getAttribute('class');

    await page.goto('/register');
    await expect(page.getByRole('heading', { name: '首次使用：設定密碼' })).toBeVisible();

    const registerCardClass = await page.locator('main section').first().getAttribute('class');
    expect(registerCardClass).toBe(loginCardClass);

    await page.getByLabel('Email').fill('user@example.com');
    await page.getByLabel('密碼').first().fill('Secret1');
    await page.getByLabel('確認密碼').fill('Secret1');

    const submitButton = page.getByRole('button', { name: '建立帳號' });
    await submitButton.click();
    await expect(submitButton).toBeDisabled();
    await page.waitForURL(/\/login$/, { timeout: 2000 });
  });
});
