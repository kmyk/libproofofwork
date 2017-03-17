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
funcname, prefix, s, cnt = stdout.splitlines()
funcname = funcname.decode()
prefix = prefix.decode().lower()
s = s.decode()
cnt = int(cnt)
assert funcname in [ 'md5', 'sha1' ]
assert all(c in '0123456789abcdef' for c in prefix)

# print result
func = { 'md5': hashlib.md5, 'sha1': hashlib.sha1 }[funcname]
digest = func(s.encode()).hexdigest().lower()
print('[*] {}({}) = {}'.format(funcname, repr(s), digest))
print('[*] {} hashes/sec'.format(cnt / elapsed))
if not digest.startswith(prefix):
    print('[ERROR] wrong result: {} for prefix {}'.format(repr(s), prefix))
    sys.exit(1)
else:
    print('[*] accepted: {} for prefix {}'.format(repr(s), prefix, elapsed))
