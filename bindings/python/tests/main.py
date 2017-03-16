import unittest
import proofofwork
import hashlib
import random

class ProofOfWorkTest(unittest.TestCase):
    def test_md5(self):
        for len_s in range(1, 6):
            s = ''.join([ random.choice('0123456789abcdef') for _ in range(len_s) ])
            prefix = bytes(bytearray([ random.randrange(256) for _ in range(random.randrange(44)) ]))
            text = proofofwork.md5(s, prefix=prefix)
            self.assertIsInstance(text, bytes)
            self.assertTrue(text.startswith(prefix))
            self.assertTrue(hashlib.md5(text).hexdigest().startswith(s))

if __name__ == '__main__':
    unittest.main()
