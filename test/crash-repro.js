'use strict'

const path = require('path')
const { Worker } = require('worker_threads')

// Preload the native module in the main thread so worker threads don't
// race on first dlopen/NAPI registration when spawned concurrently.
require('..')

const WORKER_PATH = path.join(__dirname, 'crash-repro-worker.js')
const CONCURRENCY = Number(process.env.REPRO_CONCURRENCY) || 16
const DURATION_MS = Number(process.env.REPRO_DURATION_MS) || 5000
const MIN_LIFETIME_MS = 5
const MAX_LIFETIME_MS = 50

const deadline = Date.now() + DURATION_MS
let spawned = 0
let terminated = 0
let inflight = 0

function churn () {
  if (Date.now() >= deadline) {
    if (inflight === 0) {
      console.log(`no crash after ${DURATION_MS}ms: spawned=${spawned} terminated=${terminated}`)
    }
    return
  }

  const worker = new Worker(WORKER_PATH)
  spawned++
  inflight++

  worker.on('exit', () => {
    inflight--
    terminated++
    churn()
  })

  const lifetime = MIN_LIFETIME_MS + Math.random() * (MAX_LIFETIME_MS - MIN_LIFETIME_MS)
  setTimeout(() => worker.terminate(), lifetime)
}

for (let i = 0; i < CONCURRENCY; i++) churn()
