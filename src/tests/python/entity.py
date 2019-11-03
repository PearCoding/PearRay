import unittest
import numpy as np

PR = None


class TestEntity(unittest.TestCase):
    def test_default(self):
        entity = PR.VirtualEntity(0, "TEST")

        self.assertEqual(entity.id, 0)
        self.assertEqual(entity.name, "TEST")
        self.assertEqual(entity.type, "null")

        self.assertEqual(entity.frozen, False)


def runTest(pr):
    global PR
    PR = pr

    suite = unittest.TestLoader().loadTestsFromTestCase(TestEntity)
    return unittest.TextTestRunner(verbosity=2).run(suite).wasSuccessful()



