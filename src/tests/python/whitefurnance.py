import unittest
import os
import tempfile
import numpy as np


PR = None

TESTDIR = ""
IMGSIZE = 200
SPECSCENESTR = """
(scene
    :name 'spectral_test'
    :render_width {size}
    :render_height {size}
    :spectral_domain 520

    ; Settings
    (integrator
        :type 'DIRECT'
        :max_ray_depth 4
        :light_sampe_count 1
    )
    (sampler
        :slot 'aa'
        :type 'hammersley'
        :sample_count 8
    )
    (filter
        :slot 'pixel'
        :type 'BLOCK'
        :radius 0
    )
    ; Outputs
    (output
        :name 'image'
        (channel :type 'color' :color 'srgb' )
    )
    ; Camera
    (camera
        :name 'Camera'
        :type 'orthographic'
        :width 2
        :height 2
        :local_direction [0,0,1]
        :local_up [0,1,0]
        :local_right [1,0,0]
        :position [0,0,-1.0005]
    )
    ; Background
    (light
        :name 'background'
        :type 'env'
        :radiance 1
    )
    ; Materials
    (material
        :name 'Diffuse'
        :type 'diffuse'
        :albedo 1
    )
    ; Primitives
    (entity
        :type "sphere"
        :name "Unit Sphere"
        :radius 1
        :material "Diffuse"
    )
)
"""

FULLSCENESTR = """
(scene
    :name 'illum_test'
    :render_width {size}
    :render_height {size}
    :camera 'Camera'
    :spectral_hero {hero}

    ; Settings
    (integrator
        :type 'DIRECT'
        :max_ray_depth 4
        :light_sampe_count 1
    )
    (sampler
        :slot 'aa'
        :type 'hammersley'
        :sample_count 8
    )
    (sampler
        :slot 'spectral'
        :type 'random'
        :sample_count 1
    )
    (filter
        :slot 'pixel'
        :type 'BLOCK'
        :radius 0
    )
    ; Outputs
    (output
        :name 'image'
        (channel :type 'color' :color 'srgb' )
    )
    ; Camera
    (camera
        :name 'Camera'
        :type 'orthographic'
        :width 2
        :height 2
        :local_direction [0,0,1]
        :local_up [0,1,0]
        :local_right [1,0,0]
        :position [0,0,-1.00005]
    )
    ; Background
    (light
        :name 'background'
        :type 'env'
        :radiance (illuminant "D65")
    )
    ; Materials
    (material
        :name 'Diffuse'
        :type 'diffuse'
        :albedo "white"
    )
    ; Primitives
    (entity
        :type "sphere"
        :name "Unit Sphere"
        :radius 1
        :material "Diffuse"
    )
)
"""

POINTS = [[0.50, 0.50], [0.25, 0.25], [0.75, 0.25], [0.25, 0.75], [
    0.75, 0.75], [0.05, 0.05], [0.95, 0.05], [0.05, 0.95], [0.95, 0.95]]

SPEC_RESULT = 1
CIE_RESULT = 1  # TODO: Applying CIE XYZ makes it impossible to converge to 1 -> Extract real result


class TestWhitefurnance(unittest.TestCase):
    # Test result to analtically calculated result
    def render(self, scene):
        opts = PR.SceneLoader.LoadOptions()
        opts.WorkingDir = TESTDIR.name
        opts.PluginPath = TESTDIR.name  # Expect to have embedded plugins
        env = PR.SceneLoader.loadFromString(scene, opts)

        fct = env.createRenderFactory()
        intr = env.createSelectedIntegrator()
        ctx = fct.create(intr)
        output = env.createAndAssignFrameOutputDevice(ctx)
        env.setup(ctx)

        ctx.start(8, 8)
        ctx.waitForFinish()

        return np.divide(output.spectral, output.pixelweight)

    def mean(self, img):
        intr = 0
        for i in range(IMGSIZE):
            for j in range(IMGSIZE):
                intr += img[i, j, 0]
        return intr/(IMGSIZE*IMGSIZE)

    def checkAt(self, img, fx, fy):
        res = img[int(IMGSIZE*fx), int(IMGSIZE*fy), 0]
        self.assertAlmostEqual(res, 1, places=3)

    def checkStandardPos(self, img):
        for i in range(len(POINTS)):
            self.checkAt(img, POINTS[i][0], POINTS[i][1])

    def test_spec(self):
        img = self.render(SPECSCENESTR.format(size=IMGSIZE))
        self.checkStandardPos(img)
        self.assertAlmostEqual(self.mean(img), SPEC_RESULT, places=4)

    def test_non_hero(self):
        img = self.render(FULLSCENESTR.format(
            hero="false", size=IMGSIZE))
        self.checkStandardPos(img)
        self.assertAlmostEqual(self.mean(img), CIE_RESULT, places=4)
        
    def test_full(self):
        img = self.render(FULLSCENESTR.format(
            hero="true", size=IMGSIZE))
        self.checkStandardPos(img)
        self.assertAlmostEqual(self.mean(img), CIE_RESULT, places=4)


def runTest(pr):
    global PR
    PR = pr

    global TESTDIR
    TESTDIR = tempfile.TemporaryDirectory()

    suite = unittest.TestLoader().loadTestsFromTestCase(TestWhitefurnance)
    return unittest.TextTestRunner(verbosity=2).run(suite).wasSuccessful()
