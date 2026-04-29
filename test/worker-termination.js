'use strict'

const path = require('path')
const { Worker } = require('worker_threads')

const WORKER_PATH = path.join(__dirname, 'worker.js')
const WORKER_COUNT = 10
const WORKER_LIFETIME_MS = 1000

for (let i = 0; i < WORKER_COUNT; i++) {
  const worker = new Worker(WORKER_PATH)
  setTimeout(() => worker.terminate(), WORKER_LIFETIME_MS)
}
