'use strict'

const { expect } = require('chai')
const metrics = require('..')

metrics.start()

const sab = new SharedArrayBuffer(16)
const typedArray = new Int32Array(sab)

Atomics.wait(typedArray, 0, 0, 100)

setImmediate(() => {
  const stats = metrics.stats()

  expect(stats.eventLoop.count).to.equal(1)

  // Check for 90ms instead of 100ms because it's not precise.
  expect(stats.eventLoop.min).to.be.gte(90 * 1e6).and.lte(100 * 1e7)
  expect(stats.eventLoop.max).to.be.gte(90 * 1e6).and.lte(100 * 1e7)
  expect(stats.eventLoop.sum).to.be.gte(90 * 1e6).and.lte(100 * 1e7)
})
