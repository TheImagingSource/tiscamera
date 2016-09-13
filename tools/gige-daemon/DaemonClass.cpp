

#include "DaemonClass.h"

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h> // posix file checking via stat
#include <csignal>

#include <iostream>
#include <cstdlib>


LockFile::LockFile (const std::string filename)
    : lockfile_name(filename), lock_file(nullptr), file_handle(-1)
{}


LockFile::~LockFile ()
{
    unlock();
}


bool LockFile::file_exists () const
{
    struct stat buffer;
    return (stat (lockfile_name.c_str(), &buffer) == 0);
}


bool LockFile::lock ()
{
    return create_lock_file();
}


bool LockFile::unlock ()
{
    if (is_locked())
    {
        destroy_lock_file();
    }
    return true;
}


bool LockFile::create_lock_file ()
{
    if (file_exists())
    {
        return false;
    }

    file_handle = open(lockfile_name.c_str(), O_RDWR|O_CREAT, 0644);

    // TODO
    if (file_handle < 0)
    {
        exit(1); /* can not open */
    }
    if (lockf(file_handle, F_TLOCK, 0) < 0)
    {
        exit(0); /* can not lock */
    }
    /* only first instance continues */

    char str[10];
    sprintf(str,"%d\n",getpid());
    write(file_handle, str, strlen(str)); /* record pid to lockfile */

    return true;
}


void LockFile::destroy_lock_file ()
{
    lockf(file_handle, F_ULOCK, 0);

    close(file_handle);
    file_handle = -1;
    // lock_file->close();
    // lock_file = nullptr;

    remove(lockfile_name.c_str());
}



std::string LockFile::get_file_name () const
{
    return lockfile_name;
}


bool LockFile::is_locked () const
{
    if (file_handle != -1)
    {
        return true;
    }
    return false;
}


std::string LockFile::get_file_content () const
{
    if (!file_exists())
    {
        return "";
    }

    std::ifstream ifs;
    ifs.open(lockfile_name);

    std::filebuf* pbuf = ifs.rdbuf();

    // get file size using buffer's members
    std::size_t size = pbuf->pubseekoff (0,ifs.end,ifs.in);
    pbuf->pubseekpos (0, ifs.in);

    // allocate memory to contain file data
    char buffer[size];

    // get file data
    pbuf->sgetn (buffer,size);

    std::string content = buffer;

    return content;
}


DaemonClass::DaemonClass (const std::string lock_file,
                          bool open_ports)
    : lock_file(LockFile(lock_file)), is_port_open(false)
{
    if (open_ports)
    {
        open_port();
        is_port_open = true;
    }
}


DaemonClass::~DaemonClass ()
{
    if (lock_file.is_locked())
    {
        lock_file.unlock();
    }

    close_port();
    is_port_open = false;
}


int DaemonClass::daemonize (signal_callback callback, bool fork_process)
{
    // we are already a daemon <- assuming this is a sysv type system
    // systemd has pid 1 and starts system daemons.
    // This check will cause the daemon to fail.
    // Therefor this is disabled and left as a reminder.
    // if (getppid() == 1)
    // {
    //     return -1;
    // }

    // another daemon is already running
    if (lock_file.file_exists())
    {
        return -2;
    }

    pid_t i;

    if (fork_process)
    {
        i = fork();

        if (i < 0)
        {
            return -3;
        }
        else if (i > 0)
        {
            return i;
        }


        setsid(); /* obtain a new process group */

        /* Fork off for the second time*/
        pid_t pid = fork();

        /* An error occurred */
        if(pid < 0)
        {
            exit(EXIT_FAILURE);
        }

        /* Success: Let the parent terminate */
        if(pid > 0)
        {
            exit(EXIT_SUCCESS);
        }
    }
    for (i = getdtablesize(); i >= 0; --i)
    {
        close(i); /* close all descriptors */
    }

    i = open("/dev/null",O_RDWR); dup(i); dup(i); /* handle standart I/O */

    umask(027); /* set newly created file permissions */

    chdir("/"); /* change to dir that always exists */

    // make this the only running instance
    if (!lock_file.lock())
    {
        return -4;
    }

    // register signals
    signal(SIGCHLD,SIG_IGN); /* ignore child */
    signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGHUP, callback); /* catch hangup signal */
    signal(SIGTERM, callback); /* catch kill signal */
    signal(SIGINT, callback); /* catch kill signal */

    return 0;
}


bool DaemonClass::stop_daemon ()
{
    lock_file.unlock();
}


bool DaemonClass::undaemonize ()
{
    int pid = get_daemon_pid();

    if (pid == -1)
    {
        std::cout << "No valid PID found." << std::endl;
        return false;
    }

    std::cout << "Killing daemon with PID " << pid << std::endl;

    int ret = kill(pid, SIGTERM);

    if (ret < 0)
    {
        return false;
    }

    return true;
}


bool DaemonClass::is_daemonized ()
{
    return false;
}


int DaemonClass::get_daemon_pid ()
{
    if (lock_file.file_exists())
    {
        std::string content = lock_file.get_file_content();

        return std::stoi(content);
    }

    return -1;
}


void DaemonClass::open_port ()
{

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);


    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

}


void DaemonClass::close_port ()
{}
