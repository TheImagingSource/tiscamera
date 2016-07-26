[Unit]
Description=GigE indexing daemon
After=network.target

[Service]
Type=forked
PIDFile=/var/lock/gige-daemon.lock
ExecStart=@CMAKE_INSTALL_PREFIX@/bin/gige-daemon start
ExecStop=@CMAKE_INSTALL_PREFIX@/bin/gige-daemon stop

[Install]
WantedBy=multi-user.target
