const defaultTheme = require("tailwindcss/defaultTheme");

module.exports = {
  content: ["./index.html", "./src/**/*.{vue,ts,tsx}"],
  theme: {
    extend: {
      colors: {
        brand: {
          50: "var(--color-brand-50)",
          100: "var(--color-brand-100)",
          500: "var(--color-brand-500)",
          600: "var(--color-brand-600)",
          700: "var(--color-brand-700)"
        },
        surface: {
          DEFAULT: "var(--surface-default)",
          subtle: "var(--surface-subtle)"
        },
        ink: {
          DEFAULT: "var(--ink-primary)",
          muted: "var(--ink-muted)",
          inverse: "var(--ink-inverse)"
        },
        danger: "var(--color-danger)",
        success: "var(--color-success)",
        warning: "var(--color-warning)"
      },
      boxShadow: {
        soft: "var(--shadow-md)"
      },
      fontFamily: {
        sans: ["'Inter'", "'Noto Sans TC'", ...defaultTheme.fontFamily.sans],
        mono: ["'JetBrains Mono'", ...defaultTheme.fontFamily.mono]
      },
      spacing: {
        1.5: "6px",
        2.5: "10px",
        3.5: "14px",
        4.5: "18px",
        7.5: "30px",
        11: "44px"
      }
    }
  },
  plugins: []
};
