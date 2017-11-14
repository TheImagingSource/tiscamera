

# Check wether the gst-overrides package is installed.

import gi
gi.require_version("Gst", "1.0")
from gi.repository import Gst

try:
    getattr(Gst, "_overrides_module")
except AttributeError:
    raise RuntimeError("Python gst overrides not installed! Try 'apt install python-gst-1.0' or 'apt install python3-gst-1.0'")
