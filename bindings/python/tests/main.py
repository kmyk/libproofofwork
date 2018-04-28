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
        self.snippet_random_test(proofofwork.sha1, hashlib.sha1)

    def test_sha256(self):
        self.snippet_random_test(proofofwork.sha256, hashlib.sha256)

    def snippet_random_test_full(self, mine, answer):
        digest_length = len(answer(b'').hexdigest())
        for _ in range(10):
            for len_s in range(1, 6):
                hash = '?' * random.randrange(digest_length - len_s) + ''.join([ random.choice('0123456789abcdef?') for _ in range(len_s) ])
                if random.random() < 0.4:
                    text = None
                else:
                    text = bytes(bytearray([ random.choice(bytearray(b'0123456789abcdef?')) for _ in range(20) ] + [ ord('?') ] * 4))
                if random.random() < 0.4:
                    alphabet = None
                else:
                    alphabet = bytes(bytearray([ random.randrange(256) for _ in range(64) ]))
                result = mine(hash, text=text, alphabet=alphabet)
                if alphabet is None:
                    alphabet = b'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
                self.assertIsInstance(result, bytes)
                for i, c in enumerate(result):
                    if text is not None and text[i] != b'?'[0]:
                        self.assertEqual(c, text[i])
                    else:
                        self.assertTrue(c in alphabet)
                self.assertTrue(re.search('^' + hash.replace('?', '.'), answer(result).hexdigest()))

    def test_md5_full(self):
        self.snippet_random_test_full(proofofwork.md5, hashlib.md5)

    def test_sha1_full(self):
        self.snippet_random_test_full(proofofwork.sha1, hashlib.sha1)

    def test_sha256_full(self):
        self.snippet_random_test_full(proofofwork.sha256, hashlib.sha256)

    def snippet_hand_test(self, mine, answer):
        s = '012'
        result = mine(s, text=br'\\\?????')
        self.assertIsInstance(result, bytes)
        self.assertTrue(result.startswith(br'\?'))
        self.assertEqual(len(result), 6)
        self.assertTrue(answer(result).hexdigest().startswith(s))

    def test_md5_hand(self):
        self.snippet_hand_test(proofofwork.md5, hashlib.md5)

    def test_sha1_hand(self):
        self.snippet_hand_test(proofofwork.sha1, hashlib.sha1)

    def test_sha256_hand(self):
        self.snippet_hand_test(proofofwork.sha256, hashlib.sha256)



if __name__ == '__main__':
    unittest.main()
