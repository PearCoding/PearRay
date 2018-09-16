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

        self.assertEqual(entity.position[0], 0)
        self.assertEqual(entity.position[1], 0)
        self.assertEqual(entity.position[2], 0)

        self.assertEqual(entity.scale[0], 1)
        self.assertEqual(entity.scale[1], 1)
        self.assertEqual(entity.scale[2], 1)

        self.assertEqual(entity.rotation[0], 1)
        self.assertEqual(entity.rotation[1], 0)
        self.assertEqual(entity.rotation[2], 0)
        self.assertEqual(entity.rotation[3], 0)


def runTest(pr):
    global PR
    PR = pr

    suite = unittest.TestLoader().loadTestsFromTestCase(TestEntity)
    return unittest.TextTestRunner(verbosity=2).run(suite).wasSuccessful()



