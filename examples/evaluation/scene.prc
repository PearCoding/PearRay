(scene
  (integrator
    :type 'direct'
    :max_ray_depth 6
    ;:direct false
  )
  (sampler
    :slot 'aa'
    :type 'sobol'
    :sample_count 128
  )
  (spectral_mapper
    :type 'spd'
  )
  (filter
    :slot 'pixel'
    :type 'triangle'
    :radius 0
  )
  ;:spectral_hero false
  :render_width 256
  :render_height 256
  (output
    :name 'cbox-pearray'
    (channel
      :type 'color'
      :color 'rgb'
      :gamma 'none'
      :mapper 'none'
    )
    (channel :type 'depth' )
    (channel :type 'feedback' )
  )
  :camera 'Camera'
  (camera
    :name 'Camera'
    :type 'standard'
    :near 10
    :far 2800
    ;:fov 39.3077
    :width 0.71429
    :height 0.71429
    :local_up [0 1 0]
    :local_direction [0 0 1]
    :local_right [-1 0 0]
    :position [278 273 -800]
  )
  (material
    :type 'diffuse'
    :name 'box'
    :albedo (spectrum :start 400 :end 700 0.343 0.445 0.551 0.624 0.665 0.687 0.708 0.723 0.715 0.71 0.745 0.758 0.739 0.767 0.777 0.765 0.751 0.745 0.748 0.729 0.745 0.757 0.753 0.75 0.746 0.747 0.735 0.732 0.739 0.734 0.725 0.721 0.733 0.725 0.732 0.743 0.744 0.748 0.728 0.716 0.733 0.726 0.713 0.74 0.754 0.764 0.752 0.736 0.734 0.741 0.74 0.732 0.745 0.755 0.751 0.744 0.731 0.733 0.744 0.731 0.712 0.708 0.729 0.73 0.727 0.707 0.703 0.729 0.75 0.76 0.751 0.739 0.724 0.73 0.74 0.737)
  )
  (material
    :type 'diffuse'
    :name 'white'
    :albedo (spectrum :start 400 :end 700 0.343 0.445 0.551 0.624 0.665 0.687 0.708 0.723 0.715 0.71 0.745 0.758 0.739 0.767 0.777 0.765 0.751 0.745 0.748 0.729 0.745 0.757 0.753 0.75 0.746 0.747 0.735 0.732 0.739 0.734 0.725 0.721 0.733 0.725 0.732 0.743 0.744 0.748 0.728 0.716 0.733 0.726 0.713 0.74 0.754 0.764 0.752 0.736 0.734 0.741 0.74 0.732 0.745 0.755 0.751 0.744 0.731 0.733 0.744 0.731 0.712 0.708 0.729 0.73 0.727 0.707 0.703 0.729 0.75 0.76 0.751 0.739 0.724 0.73 0.74 0.737)
  )
  (material
    :type 'diffuse'
    :name 'red'
    :albedo (spectrum :start 400 :end 700 0.04 0.046 0.048 0.053 0.049 0.05 0.053 0.055 0.057 0.056 0.059 0.057 0.061 0.061 0.06 0.062 0.062 0.062 0.061 0.062 0.06 0.059 0.057 0.058 0.058 0.058 0.056 0.055 0.056 0.059 0.057 0.055 0.059 0.059 0.058 0.059 0.061 0.061 0.063 0.063 0.067 0.068 0.072 0.08 0.09 0.099 0.124 0.154 0.192 0.255 0.287 0.349 0.402 0.443 0.487 0.513 0.558 0.584 0.62 0.606 0.609 0.651 0.612 0.61 0.65 0.638 0.627 0.62 0.63 0.628 0.642 0.639 0.657 0.639 0.635 0.642)
  )
  (material
    :type 'diffuse'
    :name 'green'
    :albedo (spectrum :start 400 :end 700 0.092 0.096 0.098 0.097 0.098 0.095 0.095 0.097 0.095 0.094 0.097 0.098 0.096 0.101 0.103 0.104 0.107 0.109 0.112 0.115 0.125 0.14 0.16 0.187 0.229 0.285 0.343 0.39 0.435 0.464 0.472 0.476 0.481 0.462 0.447 0.441 0.426 0.406 0.373 0.347 0.337 0.314 0.285 0.277 0.266 0.25 0.23 0.207 0.186 0.171 0.16 0.148 0.141 0.136 0.13 0.126 0.123 0.121 0.122 0.119 0.114 0.115 0.117 0.117 0.118 0.12 0.122 0.128 0.132 0.139 0.144 0.146 0.15 0.152 0.157 0.159)
  )
  (material
    :type 'diffuse'
    :name 'luminaire'
    :albedo (spectrum :start 400 :end 700 0.78 0.78 0.78 0.78)
  )
  (emission
    :name 'luminaire'
    :type 'diffuse'
    :radiance (spectrum :start 400 :end 700 0 8 15.6 18.4)
  )
  (embed
    :loader 'obj'
    :file 'meshes/cbox_back.obj'
    :name 'back'
  )
  (embed
    :loader 'obj'
    :file 'meshes/cbox_ceiling.obj'
    :name 'ceiling'
  )
  (embed
    :loader 'obj'
    :file 'meshes/cbox_floor.obj'
    :name 'floor'
  )
  (embed
    :loader 'obj'
    :file 'meshes/cbox_greenwall.obj'
    :name 'greenwall'
  )
  (embed
    :loader 'obj'
    :file 'meshes/cbox_redwall.obj'
    :name 'redwall'
  )
  (embed
    :loader 'obj'
    :file 'meshes/cbox_largebox.obj'
    :name 'largebox'
  )
  (embed
    :loader 'obj'
    :file 'meshes/cbox_smallbox.obj'
    :name 'smallbox'
  )
  (embed
    :loader 'obj'
    :file 'meshes/cbox_luminaire.obj'
    :name 'luminaire'
  )
  (entity
    :name 'ceiling'
    :type 'mesh'
    :materials ['white']
    :mesh 'ceiling'
  )
  (entity
    :name 'floor'
    :type 'mesh'
    :materials ['white']
    :mesh 'floor'
  )
  (entity
    :name 'back'
    :type 'mesh'
    :materials ['white']
    :mesh 'back'
  )
  (entity
    :name 'redwall'
    :type 'mesh'
    :materials ['red']
    :mesh 'redwall'
  )
  (entity
    :name 'greenwall'
    :type 'mesh'
    :materials ['green']
    :mesh 'greenwall'
  )
  (entity
    :name 'smallbox'
    :type 'mesh'
    :materials ['box']
    :mesh 'smallbox'
  )
  (entity
    :name 'largebox'
    :type 'mesh'
    :materials ['box']
    :mesh 'largebox'
  )
  (entity
    :name 'luminaire'
    :type 'mesh'
    :materials ['luminaire']
    :emission 'luminaire'
    :mesh 'luminaire'
    :position [0 -0.5 0]
  )
)
