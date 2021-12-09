#########
Lifetimes
#########

This page explains the lifetimes of various elements/properties.

GST_STATE_NULL
##############

During this state __no__ hardware will be opened.

No properties will be available.

GST_STATE_NULL -> GST_STATE_READY
#################################

Upon entering this state a device will be opened.
If the requested device cannot be found or opened a `GST_STATE_CHANGE_FAILURE` will be returned.

tcamconvert/tcamdutils/tcamdutils-cuda will be initialized.

All properties will be available.

GST_STATE_READY -> GST_STATE_PAUSED
###################################

tcambin
-------

- device-caps will be fixed and set for internal tcamsrc.
- if no device-caps are set, auto negotiation will determine caps and set them.
- if `tcam-properties-json` is set, the properties will be applied.

GST_STATE_PAUSED -> GST_STATE_PLAYING
#####################################

Start image retrieval.

GST_STATE_PLAYING -> GST_STATE_PAUSED
#####################################


GST_STATE_PAUSED -> GST_STATE_READY
###################################

Negotiated caps are reset.

tcambin
-------


GST_STATE_READY -> GST_STATE_NULL
#################################

All property object are invalid and should be discarded.
Using property functions will return `TCAM_ERROR_NO_DEVICE_OPEN`.

tcambin
-------

- All internal elements will be discarded
- The source element will be discarded
  This closes the camera.

  


  
