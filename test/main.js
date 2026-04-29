'use strict'

// Preload the native module in the main thread so worker threads don't
// race on first dlopen/NAPI registration when spawned concurrently.
require('..')

require('./worker-termination')
require('./crash-repro')
