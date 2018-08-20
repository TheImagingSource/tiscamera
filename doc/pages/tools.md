# Tools {#ToolsSection}

This section deals with all tools that are
either part of tiscamera or are indirectly used.

\subpage tcam-capture - GUI application for camera interaction

\subpage tcam-ctrl - Commandline tool for camera IO

\subpage camera-ip-conf - Commandline tool for GigE camera IP configuration

\subpage tcam-gigetool

\subpage firmware-update - Commandline tool for USB firmware updates.

\subpage gige-daemon - Daemon for GigE camera indexing

### Third-Party Tools

\subpage uvcdynctrl - Tool for automatic uvc extension application for USB cameras.


## Completion

When dealing with bash auto completion you may encounter situations in which the completion does not work as expected.

Be aware that characters like '=' tend to break to completion scripts. To circumvent this you will have to wrap the concerning string in quotation marks '"'.

    tcam-capture --serial 00001234 --format "video/x-bayer,format=gbrg,width=1280,height=720,framerate=30/1"
