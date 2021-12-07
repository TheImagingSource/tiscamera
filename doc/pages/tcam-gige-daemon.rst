.. _tcam_gige_daemon:

################
tcam-gige-daemon
################

The tcam-gige-daemon is a daemon that keeps track of available GigE devices.

Searching the network for available devices takes time,
making it preferable to have a local list available when a device is opened.

Available options
=================

.. code-block:: text

      list        - list camera names  
      list-long   - list camera names, ip, mac
      start       - start daemon and fork
        --no-fork - run daemon without forking
      stop        - stop daemon

Lock File
=========

The daemon creates a lock file to ensure no other instance is running.  
This file can be found at `/var/lock/tcam-gige-daemon.lock`.

Systemd Integration
===================

The tcam-gige-daemon comes with a service file that allows systemd to control the daemon.

The following commands may be relevant in this regard:

.. code-block:: sh

  sudo systemctl daemon-reload                      # make systemd aware of gige-daemon  
  sudo systemctl enable tcam-gige-daemon.service    # start on every boot  
  sudo systemctl start tcam-gige-daemon.service     # start the actual daemon  
  sudo systemctl status tcam-gige-daemon.service    # check if statemd say everything is ok


Restricting to certain interfaces
=================================

Per default, the tcam-gige-daemon will address all available interfaces.
To restrict the daemon to a certain interface, initialize the process with the names of the interfaces to be queried.

.. code-block:: sh

   tcam-gige-daemon start eth1 eth2 # eth0 would not be queried

This change has to be performed manually in the systemd config.
