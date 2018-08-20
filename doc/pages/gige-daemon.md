# gige-daemon {#gige-daemon}

The gige-daemon is a daemon that keeps track of available GigE devices.

Searching the network for available devices takes time,
thus it is preferable to have a local list available when a device shall be opened.

## Available options

```
list        - list camera names
list-long   - list camera names, ip, mac
start       - start daemon and fork
  --no-fork - run daemon without forking
stop        - stop daemon
```

## Lock File

The daemon creates a lock file to ensure no other instance is running.
This file can be found at `/var/lock/gige-daemon.lock`.

## Systemd Integration

The gige-daemon comes with a service file that allows systemd to control the daemon.

The following commands may be relevant in this regard:

```
sudo systemctl daemon-reload                 # make systemd aware of gige-daemon
sudo systemctl enable gige-daemon.service    # start on every boot
sudo systemctl start gige-daemon.service     # start the actual daemon
sudo systemctl status gige-daemon.service    # check if statemd say everything is ok
```

## Restricting to certain interfaces

Per default the gige-daemon will address all available interfaces.  
If you want to restrict the daemon to a certain interface initialize the process with the names of the interfaces that shall be queried.

```
sudo gige-daemon start eth2 eth1 # eth0 would not be queried
```

This change has to be manually done in the systemd config, should you wish to start the process through systemd.
