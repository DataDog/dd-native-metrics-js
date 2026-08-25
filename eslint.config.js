'use strict'

const eslint = require('@eslint/js')
const stylistic = require('@stylistic/eslint-plugin')
const importX = require('eslint-plugin-import-x')
const node = require('eslint-plugin-n')
const promise = require('eslint-plugin-promise')
const globals = require('globals')

module.exports = [
  eslint.configs.recommended,
  importX.flatConfigs.recommended,
  node.configs['flat/recommended-script'],
  promise.configs['flat/recommended'],
  stylistic.configs.customize({
    braceStyle: '1tbs',
    commaDangle: 'never',
    indent: 2,
    jsx: false,
    quotes: 'single',
    semi: false
  }),
  {
    files: ['**/*.js'],
    languageOptions: {
      ecmaVersion: 2022,
      globals: globals.node,
      sourceType: 'commonjs'
    },
    rules: {
      '@stylistic/arrow-parens': [2, 'as-needed'],
      '@stylistic/max-len': [2, 120, 2],
      '@stylistic/object-curly-spacing': [2, 'always'],
      '@stylistic/space-before-function-paren': [2, 'always'],
      'import-x/no-extraneous-dependencies': 2,
      'no-console': 2,
      'no-var': 2,
      'prefer-const': 2
    }
  },
  {
    files: ['test/**/*.js'],
    languageOptions: {
      globals: globals.mocha
    },
    rules: {
      'n/handle-callback-err': 0,
      'no-unused-expressions': 0
    }
  }
]
