'use strict'

const path = require('path')
const { Worker } = require('worker_threads')

const WORKER_PATH = path.join(__dirname, 'worker.js')
const WORKER_COUNT = 10
const WORKER_LIFETIME_MS = 1000

function run () {
  return new Promise((resolve) => {
    let remaining = WORKER_COUNT
    for (let i = 0; i < WORKER_COUNT; i++) {
      const worker = new Worker(WORKER_PATH)
      worker.on('exit', () => {
        if (--remaining === 0) resolve()
      })
      setTimeout(() => worker.terminate(), WORKER_LIFETIME_MS)
    }
  })
}

module.exports = { run }

if (require.main === module) {
  run().catch((err) => {
    console.error(err)
    process.exit(1)
  })
}
