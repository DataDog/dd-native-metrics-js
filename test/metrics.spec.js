'use strict'

const { expect } = require('chai')
const path = require('path')
const { Worker } = require('worker_threads')
const nativeMetrics = require('..')

describe('metrics', () => {
  beforeEach(() => {
    nativeMetrics.start()
  })

  afterEach(() => {
    nativeMetrics.stop()
  })

  it('should collect stats', () => {
    const stats = nativeMetrics.stats()

    expect(stats).to.have.property('cpu')
    expect(stats.cpu).to.have.property('user')
    expect(stats.cpu.user).to.be.a('number')
    expect(stats.cpu).to.have.property('system')
    expect(stats.cpu.system).to.be.a('number')

    expect(stats).to.have.property('eventLoop')
    expect(stats.eventLoop).to.have.property('min')
    expect(stats.eventLoop.min).to.be.a('number')
    expect(stats.eventLoop).to.have.property('max')
    expect(stats.eventLoop.max).to.be.a('number')
    expect(stats.eventLoop).to.have.property('sum')
    expect(stats.eventLoop.sum).to.be.a('number')
    expect(stats.eventLoop).to.have.property('avg')
    expect(stats.eventLoop.avg).to.be.a('number')
    expect(stats.eventLoop).to.have.property('count')
    expect(stats.eventLoop.count).to.be.a('number')
    expect(stats.eventLoop).to.have.property('median')
    expect(stats.eventLoop.median).to.be.a('number')
    expect(stats.eventLoop).to.have.property('p95')
    expect(stats.eventLoop.p95).to.be.a('number')

    expect(stats).to.have.property('gc')
    expect(stats.gc).to.have.property('all')
    expect(stats.gc.all).to.have.property('min')
    expect(stats.gc.all.min).to.be.a('number')
    expect(stats.gc.all).to.have.property('max')
    expect(stats.gc.all.max).to.be.a('number')
    expect(stats.gc.all).to.have.property('sum')
    expect(stats.gc.all.sum).to.be.a('number')
    expect(stats.gc.all).to.have.property('avg')
    expect(stats.gc.all.avg).to.be.a('number')
    expect(stats.gc.all).to.have.property('count')
    expect(stats.gc.all.count).to.be.a('number')
    expect(stats.gc.all).to.have.property('median')
    expect(stats.gc.all.median).to.be.a('number')
    expect(stats.gc.all).to.have.property('p95')
    expect(stats.gc.all.p95).to.be.a('number')

    expect(stats).to.have.property('heap')
    expect(stats.heap).to.have.property('spaces')
    expect(stats.heap.spaces).to.be.an('array')

    for (const space of stats.heap.spaces) {
      expect(space).to.have.property('space_name')
      expect(space.space_name).to.be.a('string')
      expect(space).to.have.property('space_size')
      expect(space.space_size).to.be.a('number')
      expect(space).to.have.property('space_used_size')
      expect(space.space_used_size).to.be.a('number')
      expect(space).to.have.property('space_available_size')
      expect(space.space_available_size).to.be.a('number')
      expect(space).to.have.property('physical_space_size')
      expect(space.physical_space_size).to.be.a('number')
    }
  })

  it('should collect CPU stats in microseconds', done => {
    for (let i = 0; i < 1e8; i++) {
      // eating delicious CPU cycles
    }

    setImmediate(() => {
      const stats = nativeMetrics.stats()

      expect(stats.cpu.user).to.be.gte(1000)
      expect(stats.cpu.user).to.be.lte(1e6)

      done()
    })
  })

  it('should collect event loop stats in nanoseconds', done => {
    const sab = new SharedArrayBuffer(16)
    const typedArray = new Int32Array(sab)

    Atomics.wait(typedArray, 0, 0, 100)

    setImmediate(() => {
      const stats = nativeMetrics.stats()

      expect(stats.eventLoop.count).to.be.gte(1)
      expect(stats.eventLoop.max).to.be.gte(100 * 1e6)
      expect(stats.eventLoop.sum).to.be.gte(100 * 1e6)

      done()
    })
  })

  it('should support worker threads', done => {
    const worker = new Worker(path.join(__dirname, 'worker.js'))

    worker.once('exit', code => {
      if (code) {
        done(new Error('Worker exited with non-zero code.'))
      } else {
        done()
      }
    })
  })
})
