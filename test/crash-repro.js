'use strict'

const path = require('path')
const { Worker } = require('worker_threads')

const WORKER_PATH = path.join(__dirname, 'crash-repro-worker.js')
const CONCURRENCY = Number(process.env.REPRO_CONCURRENCY) || 16
const DURATION_MS = Number(process.env.REPRO_DURATION_MS) || 5000
const MIN_LIFETIME_MS = 5
const MAX_LIFETIME_MS = 50

function run () {
  return new Promise((resolve, reject) => {
    const deadline = Date.now() + DURATION_MS
    let spawned = 0
    let terminated = 0
    let inflight = 0
    let settled = false

    function churn () {
      if (settled) return

      if (Date.now() >= deadline) {
        if (inflight === 0) {
          settled = true
          console.log(`no crash after ${DURATION_MS}ms: spawned=${spawned} terminated=${terminated}`)
          resolve()
        }
        return
      }

      const worker = new Worker(WORKER_PATH)
      spawned++
      inflight++

      worker.on('error', (err) => {
        if (settled) return
        settled = true
        reject(err)
      })

      worker.on('exit', () => {
        inflight--
        terminated++
        churn()
      })

      const lifetime = MIN_LIFETIME_MS + Math.random() * (MAX_LIFETIME_MS - MIN_LIFETIME_MS)
      setTimeout(() => worker.terminate(), lifetime)
    }

    for (let i = 0; i < CONCURRENCY; i++) churn()
  })
}

module.exports = { run }

if (require.main === module) {
  run().catch((err) => {
    console.error(err)
    process.exit(1)
  })
}
