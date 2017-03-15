#!/usr/bin/env python3
import hashlib
import itertools
for i in itertools.count():
    s = str(i).encode()
    digest = hashlib.md5(s).hexdigest()
    if digest.startswith('c0ffee'):
        break
print('c0ffee')
print(i)
print(i)
