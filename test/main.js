'use strict'

const path = require('path')
const { Worker } = require('worker_threads')

for (let i = 0; i < 10; i++) {
  const worker = new Worker(path.join(__dirname, 'worker.js'))

  setTimeout(() => {
    worker.terminate()
  }, 1000)
}
