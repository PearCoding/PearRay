import unittest

PR = None


class TestVector(unittest.TestCase):
    def test_get(self):
        entity = PR.Entity(0, "TEST")

        self.assertEqual(entity.position[0], 0)
        self.assertEqual(entity.position[1], 0)
        self.assertEqual(entity.position[2], 0)


def runTest(pr):
    PR = pr

    suite = unittest.TestLoader().loadTestsFromTestCase(TestVector)
    unittest.TextTestRunner(verbosity=2).run(suite)
