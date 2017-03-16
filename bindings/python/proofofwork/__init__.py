import sys
import os
import ctypes
library_path = os.path.abspath(os.path.join(os.path.dirname(sys.modules['proofofwork'].__file__), 'libproofofwork.so'))
library = ctypes.cdll.LoadLibrary(library_path)
library.pow_md5_mine.restype = ctypes.c_bool
# library.pow_md5_mine.argtypes = (ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint64))

MD5_DIGEST_LENGTH = 16
MD5_CHUNK_LENGTH = 64
prefix_length_limit = 44
def md5(s, prefix=None):
    '''
    :type s: str or None
    :type prefix: bytes or None
    :rtype: bytes
    '''
    if not isinstance(s, str):
        raise TypeError
    if len(s) > 2*MD5_DIGEST_LENGTH:
        raise ValueError
    s += '?' * (2*MD5_DIGEST_LENGTH - len(s))
    if not all(c in '0123456789abcdef?' for c in s):
        raise ValueError
    if prefix is None:
        prefix = b''
    if not isinstance(prefix, bytes):
        raise TypeError
    if len(prefix) > prefix_length_limit:
        raise ValueError
    mask   = (ctypes.c_uint8 * MD5_DIGEST_LENGTH)()
    target = (ctypes.c_uint8 * MD5_DIGEST_LENGTH)()
    for i in range(MD5_DIGEST_LENGTH):
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
    buf    = (ctypes.c_uint8 * MD5_CHUNK_LENGTH)()
    for i, c in enumerate(bytearray(prefix)):
        buf[i] = c
    size   = ctypes.c_uint64(len(prefix))
    found = library.pow_md5_mine(ctypes.byref(mask), ctypes.byref(target), ctypes.byref(buf), ctypes.byref(size))
    if found:
        return bytes(bytearray(buf[:size.value]))
    else:
        return None
