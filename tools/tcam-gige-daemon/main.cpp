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


#include "CameraListHolder.h"
#include "DaemonClass.h"
#include "gige-daemon.h"

#include <csignal>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>

using namespace tcam::tools;

namespace
{

DaemonClass daemon_instance(gige_daemon::LOCK_FILE);


std::vector<struct tcam_device_info> get_camera_list()
{

    key_t shmkey = ftok(gige_daemon::LOCK_FILE, 'G');
    key_t sem_key = ftok(gige_daemon::LOCK_FILE, 'S');

    if (sem_key < 0)
    {
        perror("Unable to get semaphore key.");
        return {};
    }

    int sem_id = tcam::semaphore_create(sem_key);

    if (sem_id < 0)
    {
        perror("Cannot create semaphore");
        return {};
    }

    int shmid;
    if ((shmid = shmget(shmkey, sizeof(gige_daemon::tcam_gige_device_list), 0644 | IPC_CREAT))
        == -1)
    {
        perror("shmget");
        exit(1);
    }

    tcam::semaphore_lock(sem_id);

    gige_daemon::tcam_gige_device_list* d = (gige_daemon::tcam_gige_device_list*)shmat(shmid, NULL, 0);

    if (d == nullptr)
    {
        shmdt(d);
        return std::vector<struct tcam_device_info>();
    }

    std::vector<struct tcam_device_info> ret;

    ret.reserve(d->device_count);

    for (unsigned int i = 0; i < d->device_count; ++i) { ret.push_back(d->devices[i]); }

    shmdt(d);

    tcam::semaphore_unlock(sem_id);

    return ret;
}


void print_camera_list()
{
    auto cam_list = get_camera_list();

    for (const auto& c : cam_list) { std::cout << c.identifier << std::endl; }
}


void print_camera_list_long()
{
    auto cam_list = get_camera_list();

    for (const auto& c : cam_list)
    {
        std::cout << c.name << " - " << c.serial_number << " - " << c.identifier << std::endl;
    }
}


void print_help(const char* prog_name)
{
    std::cout << prog_name << " - GigE Indexing daemon\n"
              << "\n"
              << "Usage:\n"
              << "\t" << prog_name << " list \t - list camera names\n"
              << "\t" << prog_name << " list-long \t - list camera names, ip, mac\n"
              << "\t" << prog_name << " start \t - start daemon and fork\n"
              << "\t\t --no-fork \t - run daemon without forking\n"
              << "\t" << prog_name << " stop \t - stop daemon\n"
              << std::endl;
}


void signal_handler(int sig)
{
    switch (sig)
    {
        case SIGHUP:
            /* rehash the server */
            break;
        case SIGTERM:
        case SIGINT:
            /* finalize the server */

            gige_daemon::CameraListHolder::get_instance().stop();
            daemon_instance.stop_daemon();

            break;
        default:
            std::cout << "Received unhandled signal " << sig << std::endl;
            break;
    }
}

} // namespace

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
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
            bool daemonize = true;

            for (int j = 1; j < argc; ++j)
            {
                if (strcmp("--no-fork", argv[j]) == 0)
                {
                    daemonize = false;
                    break;
                }
            }

            int ret = daemon_instance.daemonize(&signal_handler, daemonize);

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
            for (int x = (i + 1); x < argc; ++x)
            {
                if (strcmp("--no-fork", argv[x]) == 0)
                {
                    continue;
                }

                interfaces.push_back(argv[x]);
            }

            // block signals in this thread and subsequently
            // spawned threads
            sigset_t sigset;
            sigemptyset(&sigset);
            sigaddset(&sigset, SIGINT);
            sigaddset(&sigset, SIGTERM);
            pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

            try
            {
                gige_daemon::CameraListHolder::get_instance().set_interface_list(interfaces);
            }
            catch (std::runtime_error& e)
            {
                std::cerr << "Unable to start indexing: " << e.what() << std::endl;
                return 1;
            }

            int signum = 0;
            // wait until a signal is delivered:
            sigwait(&sigset, &signum);

            signal_handler(signum);

            return 0;
        }
        else if (strcmp("stop", argv[i]) == 0)
        {
            if (daemon_instance.undaemonize())
            {
                std::cout << "Successfully stopped daemon." << std::endl;

                return 0;
            }
            else
            {
                std::cout << "Could not stop daemon. " << strerror(errno) << std::endl;

                return 1;
            }
        }
    }

    print_help(argv[0]);

    return 0;
}
