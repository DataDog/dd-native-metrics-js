'use strict'

const metrics = require('..')

metrics.start()

function spin () {
  for (let i = 0; i < 50; i++) {
    metrics.stats()
  }
  setImmediate(spin)
}

spin()
