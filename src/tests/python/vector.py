import unittest
import numpy as np

PR = None


class TestVector(unittest.TestCase):
    def test_get(self):
        entity = PR.Entity(0, "TEST")

        self.assertEqual(entity.position[0], 0)
        self.assertEqual(entity.position[1], 0)
        self.assertEqual(entity.position[2], 0)

    def test_set(self):
        entity = PR.Entity(0, "TEST")
        entity.position = np.array([0, 0, 1])

        self.assertEqual(entity.position[0], 0)
        self.assertEqual(entity.position[1], 0)
        self.assertEqual(entity.position[2], 1)


def runTest(pr):
    global PR
    PR = pr

    suite = unittest.TestLoader().loadTestsFromTestCase(TestVector)
    return unittest.TextTestRunner(verbosity=2).run(suite).wasSuccessful()



