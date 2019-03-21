##########
uvcdynctrl
##########

uvcdynctrl is an external tool that allows the application of UVC extension units.

In most distributions the required package is named `uvcdynctrl`.

With a default installation of tiscamera no user interaction with uvcdynctrl is required.

Should the system setup require a manual call to uvcdynctrl, use the basic command:

```
uvcdynctrl --import=/path/to/extensio.xml --device=/dev/videoX
```

For more options, please refer to the man page of uvcdynctrl.
