'use strict'

const crypto = require('crypto')
const base58 = require('bs58')


module.exports.encode = (data, prefix = '00', encoding = 'hex') => {
  if (typeof data === 'string') {
    data = new Buffer(data, encoding)
  }
  if (!(data instanceof Buffer)) {
    throw new TypeError('"data" argument must be an Array of Buffers')
  }
  if (!(prefix instanceof Buffer)) {
    prefix = new Buffer(prefix, encoding)
  }
  let hash = Buffer.concat([prefix, data])
  hash = crypto.createHash('sha256').update(hash).digest()
  hash = crypto.createHash('sha256').update(hash).digest()
  hash = Buffer.concat([prefix, data,  hash.slice(0, 4)])
  return base58.encode(hash)
}

module.exports.decode = (string, encoding) => {
  const buffer = new Buffer(base58.decode(string))
  let prefix = buffer.slice(0, 2)
  let data = buffer.slice(2, -4)
  let hash = Buffer.concat([prefix, data])
  hash = crypto.createHash('sha256').update(hash).digest()
  hash = crypto.createHash('sha256').update(hash).digest()
  buffer.slice(-4).forEach((check, index) => {
    if (check !== hash[index]) {
      throw new Error('Invalid checksum')
    }
  })
  if (encoding) {
    prefix = prefix.toString(encoding)
    data = data.toString(encoding)
  }
  return { prefix, data }
}