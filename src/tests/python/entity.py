import unittest

PR = None


class TestEntity(unittest.TestCase):
    def test_default(self):
        entity = PR.ITransformable(0, "TEST")

        self.assertEqual(entity.id, 0)
        self.assertEqual(entity.name, "TEST")
        self.assertEqual(entity.type, "null")


def runTest(pr):
    global PR
    PR = pr

    suite = unittest.TestLoader().loadTestsFromTestCase(TestEntity)
    return unittest.TextTestRunner(verbosity=2).run(suite).wasSuccessful()



