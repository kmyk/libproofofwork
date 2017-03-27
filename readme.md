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

>>> s = proofofwork.md5('00000000')
>>> s
b'erno8AAA'
>>> hashlib.md5(s).hexdigest()
'0000000057c4ca67d38e6826053824bd'

>>> s = proofofwork.sha1('000??????????????????????????????????000', text=b'sha1{????????}', alphabet=bytes(range(128, 256)))
>>> s
b'sha1{\xcb\xf0\xa4\x80\x80\x80\x80\xe0}'
>>> hashlib.sha1(s).hexdigest()
'00099d6917591c54f861032fd0d0071fbc647000'
```

## Benchmark

### MD5

In my laptop environment (`Intel(R) Core(TM) i5-6200U CPU @ 2.30GHz`),

| Code           | Speed [hashes/sec] |
| -------------- | ------------------:|
| libproofofwork |          158000000 |
| C openssl      |            5890000 |
| python hashlib |             794000 |

### SHA1

In my laptop environment (`Intel(R) Core(TM) i5-6200U CPU @ 2.30GHz`),

| Code           | Speed [hases/sec] |
| -------------- | -----------------:|
| libproofofwork |          83700000 |
| C openssl      |           6360000 |
| python hashlib |            815000 |

## License

MIT License
