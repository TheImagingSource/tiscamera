import TIS
# Starter for The Imaging Source cameras in ros.
# Needs Python 3
# Please refer to https://github.com/TheImagingSource/tiscamera
# Goto: http://wiki.ros.org/gscam for info.
# Install sudo apt-get install ros-kinetic-gscam

# Open the camera. Parameters are serial number, width, height, frame rate, Color and liveview.
Tis = TIS.TIS("17719924",1920, 1080, 30, True, False)

# Start the live stream from the camera and also "rosrun"
Tis.Start_pipeline()

# Set some properties
Tis.Set_Property("Exposure Auto", True)
Tis.Set_Property("Gain Auto", True)
Tis.Set_Property("Brightness Reference", 128)


input("Press Enter to end program")

# Stop the camera pipeline.
Tis.Stop_pipeline()

print('Program ended')



