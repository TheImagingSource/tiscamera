[Unit]
Description=GigE indexing daemon
Requires=network.target
After=network.target

[Service]
Type=simple
ExecStart=@CMAKE_INSTALL_PREFIX@/bin/gige-daemon start --no-fork
ExecStop=@CMAKE_INSTALL_PREFIX@/bin/gige-daemon stop

[Install]
WantedBy=multi-user.target
