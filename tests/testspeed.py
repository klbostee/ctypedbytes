import cStringIO
import unittest
import time
import os
import ctypedbytes as typedbytes
from itertools import imap


class TestSpeed(unittest.TestCase):

    def testints(self):
        file = open("speedtest.bin", "wb")

        output = typedbytes.Output(file)
        t = time.time()
        output.writes(xrange(100000))
        print "writing ints:", time.time() - t

        file.close()
        file = open("speedtest.bin", "rb")

        input = typedbytes.Input(file)
        t = time.time()
        for record in input:
            pass
        print "reading ints:", time.time() -t

        file.close()
        os.remove("speedtest.bin")

    def teststrings(self):
        file = open("speedtest.bin", "wb")

        output = typedbytes.Output(file)
        t = time.time()
        output.writes(imap(str, xrange(100000)))
        print "writing strings:", time.time() - t

        file.close()
        file = open("speedtest.bin", "rb")

        input = typedbytes.Input(file)
        t = time.time()
        for record in input:
            pass
        print "reading strings:", time.time() -t

        file.close()
        os.remove("speedtest.bin")

    def testunicodes(self):
        file = open("speedtest.bin", "wb")

        output = typedbytes.Output(file)
        t = time.time()
        output.writes(imap(unicode, xrange(100000)))
        print "writing unicodes:", time.time() - t

        file.close()
        file = open("speedtest.bin", "rb")

        input = typedbytes.Input(file)
        t = time.time()
        for record in input:
            pass
        print "reading unicodes:", time.time() -t

        file.close()
        os.remove("speedtest.bin")


if __name__ == "__main__":
    suite = unittest.TestLoader().loadTestsFromTestCase(TestSpeed)
    unittest.TextTestRunner(verbosity=2).run(suite)
