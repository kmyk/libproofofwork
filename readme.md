# libProofOfWork

Simple hash-mining c library and its python binding.

## How to Install

``` sh
$ pip3 install proofofwork
```

## How to Use

``` python
$ python3
>>> import proofofwork
>>> s = proofofwork.md5('00000000', prefix=b'PREFIX_')
>>> s
b'PREFIX_AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA9!!!!!%|crF'

>>> import hashlib
>>> hashlib.md5(s).hexdigest()
'00000000fda2bbe35f55eca233c66b85'
```

## Benchmark

In my environment,

| Code             | Speed [hashes/sec]  |
| ---------------- | -------------------:|
| libproofofwork   |           182000000 |
| C openssl `MD5`  |             5890000 |
| python `hashlib` |              794000 |

They are on CPU.
If you have GPUs, you may be faster than above.

## Todo

-   write test
-   fix setup.py
-   support Python2
-   support SHA1, SHA2
-   support old CPU
-   support GPU

## License

MIT License
