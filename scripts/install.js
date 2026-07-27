#!/usr/bin/env node
const { spawnSync } = require('child_process')

const runtime = process.env.LIBSESSION_RUNTIME || 'electron'
const runtimeVersion = process.env.LIBSESSION_RUNTIME_VERSION || '40.0.0'

const result = spawnSync(
  'cmake-js',
  [
    'build',
    `--runtime=${runtime}`,
    `--runtime-version=${runtimeVersion}`,
    '--CDSUBMODULE_CHECK=OFF',
    '--CDLOCAL_MIRROR=https://oxen.rocks/deps',
    '--CDENABLE_NETWORKING=OFF',
    '--CDWITH_TESTS=OFF',
  ],
  { stdio: 'inherit', shell: true },
)

process.exit(result.status == null ? 1 : result.status)
