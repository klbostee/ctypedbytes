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
        print "int reads:", time.time() - t

        file.close()
        file = open("speedtest.bin", "rb")

        input = typedbytes.Input(file)
        t = time.time()
        for record in input:
            pass
        print "int reads:", time.time() -t

        file.close()
        os.remove("speedtest.bin")

    def teststrings(self):
        file = open("speedtest.bin", "wb")

        output = typedbytes.Output(file)
        t = time.time()
        output.writes(imap(str, xrange(100000)))
        print "string writes:", time.time() - t

        file.close()
        file = open("speedtest.bin", "rb")

        input = typedbytes.Input(file)
        t = time.time()
        for record in input:
            pass
        print "string reads:", time.time() -t

        file.close()
        os.remove("speedtest.bin")

    def testunicodes(self):
        file = open("speedtest.bin", "wb")

        output = typedbytes.Output(file)
        t = time.time()
        output.writes(imap(unicode, xrange(100000)))
        print "unicode writes:", time.time() - t

        file.close()
        file = open("speedtest.bin", "rb")

        input = typedbytes.Input(file)
        t = time.time()
        for record in input:
            pass
        print "unicode reads:", time.time() -t

        file.close()
        os.remove("speedtest.bin")


if __name__ == "__main__":
    suite = unittest.TestLoader().loadTestsFromTestCase(TestSpeed)
    unittest.TextTestRunner(verbosity=2).run(suite)
