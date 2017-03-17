# libProofOfWork

[![Travis](https://img.shields.io/travis/kmyk/libproofofwork.svg)](https://travis-ci.org/kmyk/libproofofwork)
[![PyPI](https://img.shields.io/pypi/l/proofofwork.svg)]()
[![PyPI](https://img.shields.io/pypi/pyversions/proofofwork.svg)]()
[![PyPI](https://img.shields.io/pypi/status/proofofwork.svg)]()
[![PyPI](https://img.shields.io/pypi/v/proofofwork.svg)]()

Simple hash-mining c library and its python binding.

## Requirements

-   `cmake` and things to build
-   non-old CPU for AVX2 instructions
    -   Check `flags` sections of your `/proc/cpuinfo`.
-   OpenMP (optional)

## How to Install

``` sh
$ pip install proofofwork
```

## How to Use

``` python
>>> import proofofwork
>>> import hashlib

>>> s = proofofwork.md5('00000000', prefix=b'PREFIX_')
>>> s
b'PREFIX_AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA9!!!!!%|crF'
>>> hashlib.md5(s).hexdigest()
'00000000fda2bbe35f55eca233c66b85'

>>> s = proofofwork.sha1('?????????????????????????0?0?0?0?0?0?0?0')
>>> s
b'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQ!!!O|_U!!/'
>>> hashlib.sha1(s).hexdigest()
'48a0b84a5e51b5bd1b7e23dc00600040f0f03080'
```

## Benchmark

### MD5

In my environment,

| Code           | Speed [hashes/sec] |
| -------------- | ------------------:|
| libproofofwork |          182000000 |
| C openssl      |            5890000 |
| python hashlib |             794000 |

### SHA1

In my environment,

| Code           | Speed [hases/sec] |
| -------------- | -----------------:|
| libproofofwork |          85400000 |
| C openssl      |           6360000 |
| python hashlib |            815000 |

They are on CPU.
If you have GPUs, you may be able to compute hashes faster than above.

## License

MIT License
