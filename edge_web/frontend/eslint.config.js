import js from '@eslint/js';
import vue from 'eslint-plugin-vue';
import vueParser from 'vue-eslint-parser';
import tsPlugin from '@typescript-eslint/eslint-plugin';
import tsParser from '@typescript-eslint/parser';
import prettier from 'eslint-config-prettier';

const projectParserOptions = {
  ecmaVersion: 'latest',
  sourceType: 'module',
  project: './tsconfig.json',
  tsconfigRootDir: import.meta.dirname,
  extraFileExtensions: ['.vue']
};

const browserGlobals = {
  window: 'readonly',
  document: 'readonly',
  console: 'readonly',
  setTimeout: 'readonly',
  clearTimeout: 'readonly',
  fetch: 'readonly',
  FormData: 'readonly',
  Headers: 'readonly',
  Request: 'readonly',
  Response: 'readonly',
  WebSocket: 'readonly',
  RTCPeerConnection: 'readonly',
  MediaStream: 'readonly',
  navigator: 'readonly',
  location: 'readonly'
};

export default [
  {
    ignores: ['dist/**', 'coverage/**', 'node_modules/**', 'tests/e2e/**', 'public/**']
  },
  {
    languageOptions: {
      globals: browserGlobals
    }
  },
  js.configs.recommended,
  {
    files: ['**/*.ts', '**/*.tsx'],
    languageOptions: {
      parser: tsParser,
      parserOptions: projectParserOptions
    },
    plugins: {
      '@typescript-eslint': tsPlugin
    },
    rules: {
      ...tsPlugin.configs.recommended.rules
    }
  },
  {
    files: ['**/*.vue'],
    languageOptions: {
      parser: vueParser,
      parserOptions: {
        ...projectParserOptions,
        parser: tsParser
      }
    },
    plugins: {
      vue,
      '@typescript-eslint': tsPlugin
    },
    rules: {
      ...vue.configs['flat/recommended'].rules,
      ...tsPlugin.configs.recommended.rules,
      'vue/multi-word-component-names': 'off'
    }
  },
  prettier,
  {
    rules: {
      'no-console': ['warn', { allow: ['error', 'warn'] }],
      'no-debugger': 'warn'
    }
  }
];
