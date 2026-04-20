'use strict'

const tests = [
  require('./worker-termination'),
  require('./crash-repro')
]

async function main () {
  for (const test of tests) {
    await test.run()
  }
}

main().catch((err) => {
  console.error(err)
  process.exit(1)
})
