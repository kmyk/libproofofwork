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
library.pow_set_alphabet.restype = ctypes.c_bool
library.pow_get_alphabet.restype = ctypes.c_bool
library.pow_get_num_threads.restype = ctypes.c_int
library.pow_get_num_threads.argtypes = ()
library.pow_set_num_threads.restype = ctypes.c_bool
library.pow_set_num_threads.argtypes = (ctypes.c_int, )
library.pow_md5_mine.restype = ctypes.c_bool
library.pow_sha1_mine.restype = ctypes.c_bool
library.pow_sha256_mine.restype = ctypes.c_bool

def _call(
        func, hash, text,
        num_threads=None, alphabet=None,
        digest_length=None, text_length_limit=None, block_length=None, indices_length=None):

    # check alphabet
    if alphabet is not None:
        if not isinstance(alphabet, bytes):
            raise TypeError
        if len(alphabet) == 0:
            return None
        alphabet = bytearray(alphabet)

    # check num_threads
    if num_threads is not None and not isinstance(num_threads, int):
        raise TypeError

    # check hash
    if not isinstance(hash, str):
        raise TypeError
    if len(hash) > 2*digest_length:
        raise ValueError
    hash += '?' * (2*digest_length - len(hash))
    if not all(c in '0123456789abcdef?' for c in hash):
        raise ValueError
    # prepare hash
    mask   = (ctypes.c_uint8 * digest_length)()
    target = (ctypes.c_uint8 * digest_length)()
    for i in range(digest_length):
        a = 0
        b = 0
        if hash[i*2] != '?':
            a |= 0xf0
            b += int(hash[i*2], 16) * 0x10
        if hash[i*2+1] != '?':
            a |= 0x0f
            b += int(hash[i*2+1], 16)
        mask[i]   = a
        target[i] = b

    # check/prepare text
    if text is None:
        text = b'????????'
    if not isinstance(text, bytes):
        raise TypeError
    text = bytearray(text)
    indices = []
    acc = []
    i = 0
    while i < len(text):
        if text[i] == ord('\\'):
            if i+1 >= len(text):
                raise ValueError
            acc += [ text[i+1] ]
            i += 2
        elif text[i] == ord('?'):
            if len(indices) < indices_length:
                indices += [ len(acc) ]
                acc += [ 0 ]
            else:
                acc += [ bytearray(alphabet or b'A')[0] ]
            i += 1
        else:
            acc += [ text[i] ]
            i += 1
    text = bytes(bytearray(acc))
    if len(text) > text_length_limit:
        raise ValueError
    if len(indices) == 0:
        return None
    while len(indices) < indices_length:
        indices += [ -1 ]
    buf = (ctypes.c_uint8 * block_length)()
    for i, c in enumerate(bytearray(text)):
        buf[i] = c
    ixbuf = (ctypes.c_int32 * indices_length)()
    for i, x in enumerate(indices):
        ixbuf[i] = x
    size = ctypes.c_uint64(len(text))

    # set alphabet
    saved_alphabet = None
    if alphabet is not None:
        # get
        salbuf = (ctypes.c_uint8 * 256)()
        salsize = ctypes.c_uint64(256)
        library.pow_get_alphabet(ctypes.byref(salbuf), ctypes.byref(salsize))
        saved_alphabet = (salbuf, salsize)
        # set
        albuf = (ctypes.c_uint8 * len(alphabet))()
        for i, c in enumerate(alphabet):
            albuf[i] = c
        if not library.pow_set_alphabet(ctypes.byref(albuf), ctypes.c_uint64(len(alphabet))):
            raise ValueError

    # set num_threads
    saved_num_threads = library.pow_get_num_threads()
    library.pow_set_num_threads(ctypes.c_int(num_threads or 0))

    # call function
    found = func(ctypes.byref(mask), ctypes.byref(target), ctypes.byref(buf), size, ctypes.byref(ixbuf))

    # restore num_threads
    if saved_num_threads:
        library.pow_set_num_threads(saved_num_threads)

    # restore alphabet
    if saved_alphabet is not None:
        salbuf, salsize = saved_alphabet
        library.pow_set_alphabet(ctypes.byref(salbuf), salsize)

    # result
    if found:
        return bytes(bytearray(buf[:size.value]))
    else:
        return None

MD5_DIGEST_LENGTH = 16
MD5_BLOCK_LENGTH = 64
md5_text_length_limit = 44
def md5(hash, text=None, num_threads=None, alphabet=None):
    '''
    :type hash: str or None
    :type text: bytes or None
    :rtype: bytes
    '''
    return _call(
            library.pow_md5_mine,
            hash, text,
            num_threads=num_threads,
            alphabet=alphabet,
            digest_length=MD5_DIGEST_LENGTH,
            block_length=MD5_BLOCK_LENGTH,
            text_length_limit=md5_text_length_limit,
            indices_length=8,
            )

SHA1_DIGEST_LENGTH = 20
SHA1_BLOCK_LENGTH = 64
sha1_text_length_limit = 44
def sha1(hash, text=None, num_threads=None, alphabet=None):
    '''
    :type hash: str or None
    :type text: bytes or None
    :rtype: bytes
    '''
    return _call(
            library.pow_sha1_mine,
            hash, text,
            num_threads=num_threads,
            alphabet=alphabet,
            digest_length=SHA1_DIGEST_LENGTH,
            block_length=SHA1_BLOCK_LENGTH,
            text_length_limit=sha1_text_length_limit,
            indices_length=8,
            )

SHA256_DIGEST_LENGTH = 32
SHA256_BLOCK_LENGTH = 64
sha256_text_length_limit = 44
def sha256(hash, text=None, num_threads=None, alphabet=None):
    '''
    :type hash: str or None
    :type text: bytes or None
    :rtype: bytes
    '''
    return _call(
            library.pow_sha256_mine,
            hash, text,
            num_threads=num_threads,
            alphabet=alphabet,
            digest_length=SHA256_DIGEST_LENGTH,
            block_length=SHA256_BLOCK_LENGTH,
            text_length_limit=sha256_text_length_limit,
            indices_length=8,
            )
