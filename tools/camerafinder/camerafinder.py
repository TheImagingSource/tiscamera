#
# Copyright 2013 The Imaging Source Europe GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import glob
import subprocess

udevadm = "/sbin/udevadm"

v4ldevs = glob.glob("/sys/class/video4linux/*")

for devpath in v4ldevs:
	devname = devpath.split("/")[-1]
	vendor = None
	model = None
	serial = None

	output = subprocess.check_output ([udevadm, "info", "--query=all", "--path=%s" % (devpath,)]).split("\n")
	for line in output:
		try:
			t,pair = line.split()
		except:
			pass
		key = None
		try:
			key, value = pair.split("=")
		except:
			continue
		if key == "ID_MODEL":
			model = value
		elif key == "ID_SERIAL_SHORT":
			serial = value
		elif key == "ID_VENDOR":
			vendor = value
			
	print "%s: Vendor=%s Model=%s Serial=%s" % (devname, vendor, model, serial)
			
