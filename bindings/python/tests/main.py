import unittest
import proofofwork
import hashlib
import random
import re

class ProofOfWorkTest(unittest.TestCase):
    def snippet_random_test(self, mine, answer):
        for len_s in range(1, 6):
            s = ''.join([ random.choice('0123456789abcdef') for _ in range(len_s) ])
            result = mine(s)
            self.assertIsInstance(result, bytes)
            self.assertTrue(answer(result).hexdigest().startswith(s))

    def test_md5(self):
        self.snippet_random_test(proofofwork.md5, hashlib.md5)

    def test_sha1(self):
        pass# self.snippet_random_test(proofofwork.sha1, hashlib.sha1)

    def snippet_random_test_full(self, mine, answer):
        for _ in range(3):
            for len_s in range(1, 6):
                hash = ''.join([ random.choice('0123456789abcdef?') for _ in range(len_s) ])
                text = bytes(bytearray([ random.choice(bytearray(b'0123456789abcdef?')) for _ in range(20) ] + [ ord('?') ] * 8))
                alphabet = bytes(bytearray([ random.randrange(256) for _ in range(64) ]))
                result = mine(hash, text=text, alphabet=alphabet)
                self.assertIsInstance(result, bytes)
                for i, c in enumerate(result):
                    if text[i] == b'?'[0]:
                        self.assertTrue(c in alphabet)
                    else:
                        self.assertEqual(c, text[i])
                self.assertTrue(re.search('^' + hash.replace('?', '.'), answer(result).hexdigest()))

    def test_md5_full(self):
        self.snippet_random_test_full(proofofwork.md5, hashlib.md5)

    def test_sha1_full(self):
        pass# self.snippet_random_test_full(proofofwork.sha1, hashlib.sha1)


if __name__ == '__main__':
    unittest.main()
