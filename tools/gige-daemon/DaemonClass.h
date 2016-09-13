/*
 * Copyright 2016 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fstream> // filebuf

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class LockFile
{
public:
    LockFile (const std::string filename);

    ~LockFile ();

    bool file_exists () const;

    bool lock ();

    bool unlock ();

    std::string get_file_name () const;

    bool is_locked () const;

    std::string get_file_content () const;

private:

    std::string lockfile_name;
    std::filebuf* lock_file;
    int file_handle;

    bool create_lock_file ();

    void destroy_lock_file ();

};


typedef void (*signal_callback)(int signal);

class DaemonClass
{
public:

    DaemonClass (const std::string lock_file,
                bool open_ports=true);

    ~DaemonClass ();

    // forks the process and lets the process continue in the background
    // returns:
    //   -1 - unable to fork process
    //    0 - code is running the forked instance
    //  > 0 - PID of the forked instace; you are running the original caller
    // @param fork_process - true if fork shall be called
    int daemonize (signal_callback, bool fork_process);

    // stop all daemon functions
    // should only be called from the forked process
    bool stop_daemon ();

    // kills the daemon process by sending SIGTERM to the forked
    // process. Has to be handled in the user defined
    // signal_callback
    bool undaemonize ();

    // checks if a daemon instance is already running
    // only works when same lock_file is used
    bool is_daemonized ();

    // return pid of the daemon process
    int get_daemon_pid ();

private:

    static void handle_signal (int sig);

    void open_port ();

    void close_port ();

    LockFile lock_file;

    // socket related member
    bool is_port_open;

    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    char sendBuff[1025];
    time_t ticks;
};
