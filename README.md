Cinder application for deferred rendering experiments (lighing, shadows mapping, SSAO)

Forked from original code by
Anthony Scavarelli <anthony.scavarelli@gmail.com> 
at
https://github.com/PlumCantaloupe/Cinder-Deferred-Renderer

Thanks to Anthony and the contributors his code is based on.

Original code ported to c++11 and optimized / fixed a couple of things, plus some new features/controls

This code is based on a deferred renderer for point lights and screen space ambient occlusion (SSAO), including shadow mapping.

Controls
- keys 0-9 toggle through deferred layers (depth, colour, normal, shadows etc.)
- key 0 shows final composed scene
- use cursors to move current light (with shift for up/down)
- key '.' selects next light for control
- key ',' selects prevous light for control
- key 'a' toggles ambient occlusion
- key 's' toggles shadows
- key 'd' toggles disco mode

Questions/suggestions/contact: Stefan Müller Arisona - robot@arisona.ch

NOTE: tested only on Mac OS X / Xcode. Visual studio settings probably needs some fixes.
