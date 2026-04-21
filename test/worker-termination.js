'use strict'

const path = require('path')
const { Worker } = require('worker_threads')

// Preload the native module in the main thread so worker threads don't
// race on first dlopen/NAPI registration when spawned concurrently.
require('..')

const WORKER_PATH = path.join(__dirname, 'worker.js')
const WORKER_COUNT = 10
const WORKER_LIFETIME_MS = 1000

for (let i = 0; i < WORKER_COUNT; i++) {
  const worker = new Worker(WORKER_PATH)
  setTimeout(() => worker.terminate(), WORKER_LIFETIME_MS)
}
