import unittest
import os
import tempfile
import numpy as np


PR = None

# Rectangle definition
Rx = np.array([1, 0, 0])
Ry = np.array([0, 1, 0])
Ro = np.array([0, 0, 2])
Np = np.array([0, 0, 1])


def norm(k):
    # Utility functions
    return k / np.linalg.norm(k)


def analytical(p):
    # Analytical solution
    R0 = Ro - [p[0], p[1], 0]
    R1 = R0 + Rx
    R2 = R0 + Rx + Ry
    R3 = R0 + Ry

    IA = 0.5 / np.pi
    b0 = np.absolute(np.arccos(np.dot(norm(R0), norm(R1))))
    b1 = np.absolute(np.arccos(np.dot(norm(R1), norm(R2))))
    b2 = np.absolute(np.arccos(np.dot(norm(R2), norm(R3))))
    b3 = np.absolute(np.arccos(np.dot(norm(R3), norm(R0))))
    K0 = norm(np.cross(R0, R1))
    K1 = norm(np.cross(R1, R2))
    K2 = norm(np.cross(R2, R3))
    K3 = norm(np.cross(R3, R0))
    K = b0*K0 + b1*K1 + b2*K2 + b3*K3
    return IA*(Np[0]*K[0] + Np[1]*K[1] + Np[2]*K[2])


TESTDIR = ""
IMGSIZE = 200
SCENESTR = """
(scene
    :name 'validity_test'
    :render_width {size}
    :render_height {size}
    :spectral_domain 520

    (integrator
      :type '{integrator}'
      :max_ray_depth 1
      :light_sample_count 1
    )
    (sampler
      :slot 'aa'
      :type 'hammersley'
      :sample_count 64
    )
    (filter
      :slot 'pixel'
      :type 'mitchell'
      :radius 0
    )
    ; Outputs
    (output
        :name 'image'
        (channel :type 'color' :color 'xyz' )
    )
    ; Camera
    (camera
        :name 'Camera'
        :type 'orthographic'
        :localDirection [0, 0,-1]
        :localUp [0, 1, 0]
        :localRight [1, 0, 0]
        :position [0, 0, 1]
    )

    ; Light Area
    (emission
        :name 'Area_em'
        :type 'diffuse'
        :radiance (illuminant "E")
    )
    (entity
        :name 'Area'
        :type 'plane'
        :centering true
        :width 1
        :height -1
        :emission 'Area_em'
        :position [0, 0, 2]
    )
    ; Pure diffuse object
    (material
        :name 'Diffuse'
        :type 'diffuse'
        :albedo 1
    )
    (entity
        :name 'Plane'
        :type 'plane'
        :centering true
        :material 'Diffuse'
    )
)
"""

POINTS = [[0.50, 0.50], [0.25, 0.25], [0.75, 0.25], [0.25, 0.75], [0.75, 0.75]]


# Test result to analtically calculated result
class TestDirect(unittest.TestCase):
    def render(self, scene):
        opts = PR.SceneLoader.LoadOptions()
        opts.WorkingDir = TESTDIR.name
        opts.PluginPath = TESTDIR.name  # Expect to have embedded plugins
        env = PR.SceneLoader.loadFromString(scene, opts)

        fct = env.createRenderFactory()
        intr = env.createSelectedIntegrator()
        ctx = fct.create(intr)
        env.setup(ctx)

        ctx.start(8, 8)
        ctx.waitForFinish()
        
        self.assertAlmostEqual(np.average(ctx.output.pixelweight), 64, places=3)

        return np.divide(ctx.output.spectral, ctx.output.pixelweight)

    def checkAt(self, img, points):
        for i in range(len(points)):
            res = img[int(IMGSIZE*points[i][0]), int(IMGSIZE*points[i][1]), 0]
            expected = analytical(points[i])
            self.assertAlmostEqual(res, expected, places=3)

    def checkIntegrator(self, integrator):
        img = self.render(SCENESTR.format(integrator=integrator, size=IMGSIZE))
        self.checkAt(img, POINTS)

    def test_direct(self):
        self.checkIntegrator("direct")

    def test_bidirect(self):
        self.checkIntegrator("bidi")

    def test_ppm(self):
        self.checkIntegrator("ppm")


def runTest(pr):
    global PR
    PR = pr

    global TESTDIR
    TESTDIR = tempfile.TemporaryDirectory()

    suite = unittest.TestLoader().loadTestsFromTestCase(TestDirect)
    return unittest.TextTestRunner(verbosity=2).run(suite).wasSuccessful()
