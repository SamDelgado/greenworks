'use strict'

const path = require('path')

// Allow setting a different steamworks_sdk path via an environment variable.
if (process.env.STEAMWORKS_SDK_PATH) {
  console.log(process.env.STEAMWORKS_SDK_PATH)
} else {
  // Otherwise, use the default path (project_root/steamworks_sdk)
  console.log(path.join(__dirname, '..', '..', '..', 'steamworks_sdk'))
}
