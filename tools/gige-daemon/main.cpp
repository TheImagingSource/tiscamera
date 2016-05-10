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


#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>


#include <stdio.h>
#include <iostream>
#include <cstring>
#include <csignal>

#include <unistd.h>

#include "CameraListHolder.h"
#include "DaemonClass.h"

#include "gige-daemon.h"
#include "tcam-semaphores.h"

static const std::string RUNNING_DIR = "/";
static const std::string LOCK_FILE = "/var/lock/gige-daemon.lock";
static const std::string LOG_FILE = "gige-daemon.log";

DaemonClass daemon_instance (LOCK_FILE);


std::vector<struct tcam_device_info> get_camera_list ()
{

    key_t shmkey = ftok("/tmp/tcam-gige-camera-list", 'G');
    key_t sem_key = ftok("/tmp/tcam-gige-semaphore", 'S');

    int sem_id = tcam::semaphore_create(sem_key);

    int shmid;
    if ((shmid = shmget(shmkey, sizeof(struct tcam_gige_device_list), 0644 | IPC_CREAT)) == -1)
    {
        perror("shmget");
        exit(1);
    }

    tcam::semaphore_lock(sem_id);

    struct tcam_gige_device_list* d = (struct tcam_gige_device_list*)shmat(shmid, NULL, NULL);

    if (d == nullptr)
    {
        shmdt(d);
        return std::vector<struct tcam_device_info>();
    }

    std::vector<struct tcam_device_info> ret;

    ret.reserve(d->device_count);

    for (unsigned int i = 0; i < d->device_count; ++i)
    {
        ret.push_back(d->devices[i]);
    }

    shmdt(d);

    tcam::semaphore_unlock(sem_id);

    return ret;
}


void print_camera_list ()
{
    auto cam_list = get_camera_list();

    for (const auto& c : cam_list)
    {
        std::cout << c.identifier << std::endl;
    }
}


void print_camera_list_long ()
{
    auto cam_list = get_camera_list();

    for (const auto& c : cam_list)
    {
        std::cout << c.name << " - " << c.serial_number << " - " << c.identifier << std::endl;
    }
}


void print_help (const char* prog_name)
{
    std::cout << sizeof(struct tcam_gige_device_list) << std::endl;

    std::cout << prog_name <<" - GigE Indexing daemon\n"
              << "\n"
              << "Usage:\n"
              << "\t" << prog_name << " list\n"
              << "\t" << prog_name << " list-long\n"
              << "\t" << prog_name << " start\n"
              << "\t" << prog_name << " stop\n"
              <<std::endl;
}


void signal_handler (int sig)
{
    switch (sig)
    {
        case SIGHUP:
            /* rehash the server */
            break;
        case SIGTERM:
            /* finalize the server */
            daemon_instance.stop_daemon();
            CameraListHolder::get_instance().stop();
            exit(0);
            break;
    }
}


int main (int argc, char *argv[])
{
    for (unsigned int i = 1; i < argc; ++i)
    {
        if (strcmp("list", argv[i]) == 0)
        {
            print_camera_list();
            return 0;
        }
        else if (strcmp("list-long", argv[i]) == 0)
        {
            print_camera_list_long();
            return 0;
        }
        else if (strcmp("start", argv[i]) == 0)
        {
            int ret = daemon_instance.daemonize(&signal_handler);

            if (ret < 0)
            {
                std::cerr << "Daemon is already running!" << std::endl;
                return 1;
            }

            if (ret > 0)
            {
                std::cout << "Started daemon. PID: " << ret << std::endl;
                return 0;
            }
            std::vector<std::string> interfaces;
            for (unsigned int x = i; x < argc; ++x)
            {
                interfaces.push_back(argv[x]);
            }
            CameraListHolder::get_instance().set_interface_list(interfaces);
            CameraListHolder::get_instance().run();

            while (true)
            {
                sleep(5);
            }
        }
        else if (strcmp("stop", argv[i]) == 0)
        {
            daemon_instance.undaemonize();
            return 0;
        }

    }

    print_help(argv[0]);

    return 0;
}
