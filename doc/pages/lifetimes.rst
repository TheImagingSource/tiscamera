.. _lifetimes:

#########
Lifetimes
#########

.. graphviz::

   digraph SEQ_DIAGRAM {
     graph [overlap=true, splines=line, nodesep=1.0, ordering=out];
     edge [arrowhead=none];
     node [shape=none, width=0, height=0, label=""];

     {
       rank=same;
       node[shape=rectangle, height=0.7, width=2];
       state_null[label="State NULL"];
       state_ready[label="State READY"];
       api_c[label="State PAUSED"];
       state_play[label="State PLAYING"];
     }
     // Draw vertical lines
     {
       edge [style=dashed, weight=6];
       state_null -> a1 -> a2 -> a3;
       a3 -> a4;
       a4 -> a5 -> a6;
       // a5 -> a6;
     }
     {
       edge [style=dashed, weight=6];
       state_ready -> b1;
       b1 -> b2 [penwidth=5, style=solid];
       b2 -> b3 -> b4;
       b4 -> b5 ;
       b5 -> b6[penwidth=5; style=solid];
     }
     {
       edge [style=dashed, weight=6];
       api_c -> c1 -> c2;
       c2 -> c3 [penwidth=5, style=solid];
       c3 -> c4;
       c4 -> c5 [penwidth=5, style=solid];
       c5 -> c6 -> c7;
     }
     {
       edge [style=dashed, weight=6];
       state_play -> d1 -> d2 -> d3 
       d3 -> d4 [penwidth=5, style=solid];
       d4 -> d5 -> d6;
     }
     // Draws activations
     {
       rank=same; a1 -> b1 [label="open-device", arrowhead=normal];
     }
     { rank=same; a2 -> b2 [style=invis]; b2 -> c2 [label="configuration", arrowhead=normal]; }
     
     { rank=same; c3 -> d3 [arrowhead=normal, label="PLAY()"]; }
     { rank=same; c4 -> d4 [arrowhead=normal, label="PAUSE()", dir=back]; }
     { rank=same; b5 -> c5 [arrowhead=normal, label="READY()", dir=back]; }
     { rank=same; a6 -> b6 [label="close-device", arrowhead=normal, dir=back]; }
   }


   
This page explains the lifetimes of various elements/properties.

GST_STATE_NULL
##############

During this state __no__ hardware will be opened.

No properties will be available.

GST_STATE_NULL -> GST_STATE_READY
#################################

Upon entering this state a device will be opened by the tcamsrc.
If the requested device cannot be found or opened a `GST_STATE_CHANGE_FAILURE` will be returned.

tcamconvert/tcamdutils/tcamdutils-cuda will be initialized.

All properties will be available.

GST_STATE_READY -> GST_STATE_PAUSED
###################################

- tcambin `device-caps` will be fixed and set for the internal tcamsrc.
  - if no device-caps are set, auto negotiation will determine caps and set them.
- if tcambin `tcam-properties-json` is set, the properties will be applied.

GST_STATE_PAUSED -> GST_STATE_PLAYING
#####################################

Start image retrieval.

GST_STATE_PLAYING -> GST_STATE_PAUSED
#####################################


GST_STATE_PAUSED -> GST_STATE_READY
###################################


GST_STATE_READY -> GST_STATE_NULL
#################################

- All property objects are invalid and should be discarded.
  Using property functions will return `TCAM_ERROR_NO_DEVICE_OPEN`.
- All tcambin internal elements will be discarded.
- The tcambin source element will be discarded.
  This closes the camera.
- The tcamsrc will close the hardware device.


  
