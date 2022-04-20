'use strict'

/* eslint-disable no-console */

const AdmZip = require('adm-zip')
const axios = require('axios')
const fs = require('fs')
const os = require('os')
const path = require('path')
const rimraf = require('rimraf')
const exec = require('./helpers/exec')
const title = require('./helpers/title')

title('Downloading and compiling files for release.')

const revision = exec.pipe('git rev-parse HEAD')

console.log(revision)

const branch = exec.pipe('git symbolic-ref --short HEAD')

console.log(branch)

const { GITHUB_TOKEN } = process.env

if (!GITHUB_TOKEN) {
  // eslint-disable-next-line max-len
  throw new Error('The GITHUB_TOKEN environment variable must be set to a personal access token with the `public_repo` scope to download artifacts.')
}

const client = axios.create({
  baseURL: 'https://api.github.com/',
  timeout: 5000,
  headers: {
    Authorization: `token ${GITHUB_TOKEN}`
  }
})

const fetch = (url, options) => {
  console.log(`GET ${url}`)

  return client.get(url, options)
    .catch(() => client.get(url, options))
    .catch(() => client.get(url, options))
}

getWorkflow()
  .then(getArtifact)
  .then(downloadArtifact)
  .then(extractPrebuilds)
  .catch(e => {
    process.exitCode = 1
    console.error(e)
  })

function getWorkflow () {
  return fetch(`/repos/DataDog/dd-native-metrics-js/actions/workflows/build.yml/runs?branch=${branch}`)
    .then(response => {
      const workflow = response.data.workflow_runs
        .filter(item => item.event === 'push')
        .find(item => item.head_sha === revision)

      if (!workflow) {
        throw new Error(`Unable to find workflow for ${branch}@${revision}.`)
      }

      if (workflow.status !== 'completed') {
        throw new Error(`Workflow ${workflow.id} is still running.`)
      }

      if (workflow.conclusion !== 'success') {
        throw new Error(`Aborting because workflow ${workflow.id} did not succeed.`)
      }

      return workflow
    })
}

function getArtifact (workflow) {
  return fetch(`/repos/DataDog/dd-native-metrics-js/actions/runs/${workflow.id}/artifacts`)
    .then(response => {
      const artifact = response.data.artifacts
        .find(artifact => artifact.name === 'prebuilds')

      if (!artifact) {
        throw new Error(`Missing artifacts in workflow ${workflow.id}.`)
      }

      return artifact
    })
}

function downloadArtifact (artifact) {
  return fetch(`/repos/DataDog/dd-native-metrics-js/actions/artifacts/${artifact.id}/zip`, { responseType: 'stream' })
    .then(response => {
      const destination = path.join(os.tmpdir(), 'prebuilds.zip')

      return new Promise((resolve, reject) => {
        response.data.pipe(fs.createWriteStream(destination))
          .on('finish', () => resolve())
          .on('error', reject)
      })
    })
}

function extractPrebuilds () {
  rimraf.sync('prebuilds')

  const filename = path.join(os.tmpdir(), 'prebuilds.zip')
  const target = path.join(__dirname, '..', 'prebuilds')
  const zip = new AdmZip(filename)

  zip.extractAllTo(target, true)
}
