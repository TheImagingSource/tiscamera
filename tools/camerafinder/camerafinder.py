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
			
