import sys
import os
import ctypes

soext = {
    'linux': 'so',
    'linux2': 'so',
    'darwin': 'dylib',
}
library_path = os.path.abspath(os.path.join(os.path.dirname(sys.modules['proofofwork'].__file__), 'libproofofwork.'+soext[sys.platform]))
library = ctypes.cdll.LoadLibrary(library_path)
library.pow_md5_mine.restype = ctypes.c_bool
# library.pow_md5_mine.argtypes = (ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint64))
library.pow_sha1_mine.restype = ctypes.c_bool
# library.pow_sha1_mine.argtypes = (ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint64))

def _mine_with_prefix(func, info, s, prefix):
    if not isinstance(s, str):
        raise TypeError
    if len(s) > 2*info['digest_length']:
        raise ValueError
    s += '?' * (2*info['digest_length'] - len(s))
    if not all(c in '0123456789abcdef?' for c in s):
        raise ValueError
    if prefix is None:
        prefix = b''
    if not isinstance(prefix, bytes):
        raise TypeError
    if len(prefix) > info['prefix_length_limit']:
        raise ValueError
    mask   = (ctypes.c_uint8 * info['digest_length'])()
    target = (ctypes.c_uint8 * info['digest_length'])()
    for i in range(info['digest_length']):
        a = 0
        b = 0
        if s[i*2] != '?':
            a |= 0xf0
            b += int(s[i*2], 16) * 0x10
        if s[i*2+1] != '?':
            a |= 0x0f
            b += int(s[i*2+1], 16)
        mask[i]   = a
        target[i] = b
    buf = (ctypes.c_uint8 * info['chunk_length'])()
    for i, c in enumerate(bytearray(prefix)):
        buf[i] = c
    size   = ctypes.c_uint64(len(prefix))
    found = func(ctypes.byref(mask), ctypes.byref(target), ctypes.byref(buf), ctypes.byref(size))
    if found:
        return bytes(bytearray(buf[:size.value]))
    else:
        return None

MD5_DIGEST_LENGTH = 16
MD5_CHUNK_LENGTH = 64
md5_prefix_length_limit = 44
def md5(s, prefix=None):
    '''
    :type s: str or None
    :type prefix: bytes or None
    :rtype: bytes
    '''
    return _mine_with_prefix(library.pow_md5_mine, {
            'digest_length': MD5_DIGEST_LENGTH,
            'chunk_length': MD5_CHUNK_LENGTH,
            'prefix_length_limit': md5_prefix_length_limit,
        }, s, prefix)

SHA_DIGEST_LENGTH = 20
SHA_CHUNK_LENGTH = 64
sha1_prefix_length_limit = 44
def sha1(s, prefix=None):
    '''
    :type s: str or None
    :type prefix: bytes or None
    :rtype: bytes
    '''
    return _mine_with_prefix(library.pow_sha1_mine, {
            'digest_length': SHA_DIGEST_LENGTH,
            'chunk_length': SHA_CHUNK_LENGTH,
            'prefix_length_limit': sha1_prefix_length_limit,
        }, s, prefix)
