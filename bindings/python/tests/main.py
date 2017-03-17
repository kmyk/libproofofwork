import unittest
import proofofwork
import hashlib
import random

class ProofOfWorkTest(unittest.TestCase):
    def snippet_random_test(self, mine, answer):
        for len_s in range(1, 6):
            s = ''.join([ random.choice('0123456789abcdef') for _ in range(len_s) ])
            prefix = bytes(bytearray([ random.randrange(256) for _ in range(random.randrange(44)) ]))
            text = mine(s, prefix=prefix)
            self.assertIsInstance(text, bytes)
            self.assertTrue(text.startswith(prefix))
            self.assertTrue(answer(text).hexdigest().startswith(s))

    def test_md5(self):
        self.snippet_random_test(proofofwork.md5, hashlib.md5)

    def test_sha1(self):
        self.snippet_random_test(proofofwork.sha1, hashlib.sha1)

if __name__ == '__main__':
    unittest.main()
