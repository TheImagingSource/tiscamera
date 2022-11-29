#######
Systemd
#######

systemd is a system and service manager for Linux operating systems.

It's purpose is to automate the initialization of processes without the need for user interaction.

The systemd service files are generated during the build process.

The base files can be found in `data/systemd`.

The default installation path is `/lib/systemd/system/`.

The following commands may be relevant in this regard:

.. code-block:: sh

   sudo systemctl daemon-reload                 # make systemd aware of gige-daemon  
   sudo systemctl enable tcam-gige-daemon.service    # start on every boot
   sudo systemctl start tcam-gige-daemon.service     # start the actual daemon
   sudo systemctl status tcam-gige-daemon.service    # check if statemd say everything is ok
