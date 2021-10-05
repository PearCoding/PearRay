(scene
	:name 'radiosity_test'
	:render_width 200
	:render_height 200
	;:spectral_domain 520

	(integrator 
	  :type 'direct'
	  :max_ray_depth 1
	)
  (spectral_mapper
    :type 'random'
  )
	(sampler 
	  :slot 'aa'
	  :type 'hammersley'
	  :sample_count 32
	)
	(filter 
	  :slot 'pixel'
	  :type 'mitchell'
	  :radius 0
	)
	; Outputs
	(output
		:name 'image'
		(channel :type 'color' :color 'rgb' )
		(channel :type 'var' )
	)
	; Camera
	(camera
		:name 'Camera'
		:type 'ortho'

		:width 1
		:height 1
        :local_direction [0, 0, -1]
        :local_up [0, 1, 0]
        :local_right [1, 0, 0]
        :position [0, 0, 1]
	)

    ; Light Area
    (material
        :name 'Black'
        :type 'diffuse'
        :albedo 0
    )
    (emission
        :name 'Area_em'
        :type 'diffuse'
        :radiance (smul (illuminant "D65") 10)
    )
    (entity
        :name 'Area'
        :type 'plane'
        :centering true
        :width 1
        :height -1
        :emission 'Area_em'
        :material 'Black'
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
        :position [0, 0, 0]
    )
)

