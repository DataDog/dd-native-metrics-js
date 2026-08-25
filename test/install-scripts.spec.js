'use strict'

const assert = require('assert')
const pkg = require('../package.json')

describe('package manifest', () => {
  it('declares no npm build lifecycle scripts', () => {
    const scripts = pkg.scripts || {}
    const hooks = ['preinstall', 'install', 'postinstall']
    const present = hooks.filter(name => scripts[name] !== undefined)
    assert.deepStrictEqual(
      present,
      [],
      'package.json must not declare npm build lifecycle scripts (they trigger Yarn Berry YN0007)'
    )
  })

  it('opts out of npm implicit node-gyp rebuilds', () => {
    assert.strictEqual(pkg.gypfile, false)
  })
})
