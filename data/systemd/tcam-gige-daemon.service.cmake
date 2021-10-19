[Unit]
Description=GigE indexing daemon
Requires=network.target
After=network.target

[Service]
PIDFile=/var/lock/tcam-gige-daemon.lock
Type=simple
ExecStart=@TCAM_INSTALL_BIN@/tcam-gige-daemon start --no-fork
ExecStop=@TCAM_INSTALL_BIN@/tcam-gige-daemon stop

[Install]
WantedBy=multi-user.target
