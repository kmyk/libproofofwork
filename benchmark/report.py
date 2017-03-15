#!/usr/bin/env python3
import argparse
import hashlib
import subprocess
import time
import sys

# parse args
parser = argparse.ArgumentParser()
parser.add_argument('command', nargs=argparse.REMAINDER)
args = parser.parse_args()

# run command
print('[*] run: {}'.format(args.command))
begin = time.time()
proc = subprocess.Popen(args=args.command, stdout=subprocess.PIPE, stderr=sys.stderr)
stdout, _ = proc.communicate()
end = time.time()
elapsed = end - begin
print('[*] done: {} sec elapsed'.format(elapsed))

# recieve output
prefix, s, cnt = stdout.splitlines()
prefix = prefix.decode().lower()
s = s.decode()
cnt = int(cnt)
assert all(c in '0123456789abcdef' for c in prefix)

# print result
digest = hashlib.md5(s.encode()).hexdigest().lower()
print('[*] md5({}) = {}'.format(repr(s), digest))
if not digest.startswith(prefix):
    print('[ERROR] wrong result: {} for prefix {}'.format(repr(s), prefix))
    sys.exit(1)
print('[*] accepted: {} for prefix {}'.format(repr(s), prefix, elapsed))
print('[*] {} hashes/sec'.format(cnt / elapsed))
