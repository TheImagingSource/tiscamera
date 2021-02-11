

#include "DaemonClass.h"

#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <sys/stat.h> // posix file checking via stat
#include <unistd.h>
#include <vector>


LockFile::LockFile(const std::string& filename)
    : lockfile_name_(filename), lock_file_(nullptr), file_handle_(-1)
{
}


LockFile::~LockFile()
{
    unlock();
}


bool LockFile::file_exists() const
{
    struct stat buffer;
    return (stat(lockfile_name_.c_str(), &buffer) == 0);
}


bool LockFile::lock()
{
    return create_lock_file();
}


bool LockFile::unlock()
{
    if (is_locked())
    {
        destroy_lock_file();
    }
    return true;
}


bool LockFile::create_lock_file()
{
    if (file_exists())
    {
        return false;
    }

    file_handle_ = open(lockfile_name_.c_str(), O_RDWR | O_CREAT, 0644);

    // TODO
    if (file_handle_ < 0)
    {
        exit(1); /* can not open */
    }
    if (lockf(file_handle_, F_TLOCK, 0) < 0)
    {
        std::cerr << "Unable to lock PID file" << std::endl;
        exit(0); /* can not lock */
    }
    /* only first instance continues */

    char str[10];
    sprintf(str, "%d\n", getpid());
    size_t write_ret = write(file_handle_, str, strlen(str)); /* record pid to lockfile */

    if (write_ret != strlen(str))
    {
        std::cerr << "Error while writing lockfile. Wrote " << write_ret << " bytes, but expected "
                  << strlen(str) << " bytes." << std::endl;
        return false;
    }

    // do not close file handle. this is to ensure exclusive write access

    return true;
}


void LockFile::destroy_lock_file()
{
    lockf(file_handle_, F_ULOCK, 0);

    close(file_handle_);
    file_handle_ = -1;
    // lock_file->close();
    // lock_file = nullptr;

    int ret = remove(lockfile_name_.c_str());

    if (ret < 1)
    {
        std::cerr << "Unable to delete lock file: " << strerror(errno) << std::endl;
    }
}


std::string LockFile::get_file_name() const
{
    return lockfile_name_;
}


bool LockFile::is_locked() const
{
    if (file_handle_ != -1)
    {
        return true;
    }
    return false;
}


std::string LockFile::get_file_content() const
{
    if (!file_exists())
    {
        return "";
    }

    std::ifstream ifs;
    ifs.open(lockfile_name_);

    std::filebuf* pbuf = ifs.rdbuf();

    // get file size using buffer's members
    std::size_t size = pbuf->pubseekoff(0, ifs.end, ifs.in);
    pbuf->pubseekpos(0, ifs.in);

    std::vector<char> buffer;
    buffer.reserve(size);

    // get file data
    pbuf->sgetn(buffer.data(), size);

    std::string content = buffer.data();

    return content;
}


DaemonClass::DaemonClass(const std::string& lock_file, bool open_ports)
    : lock_file_(LockFile(lock_file)), is_port_open_(false)
{
    if (open_ports)
    {
        open_port();
        is_port_open_ = true;
    }
}


DaemonClass::~DaemonClass()
{
    close_port();
    is_port_open_ = false;
}


int DaemonClass::daemonize(signal_callback callback, bool fork_process)
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
    if (lock_file_.file_exists())
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
        if (pid < 0)
        {
            exit(EXIT_FAILURE);
        }

        /* Success: Let the parent terminate */
        if (pid > 0)
        {
            exit(EXIT_SUCCESS);
        }
    }

    for (i = getdtablesize(); i >= 0; --i) { close(i); /* close all descriptors */ }

    i = open("/dev/null", O_RDWR);
    // handle warnings for -Wunused_result
    if (dup(i) != -1) {}
    if (dup(i) != -1) {}

    /* handle standart I/O */

    umask(022); /* set newly created file permissions */

    /* change to dir that always exists */
    if (chdir("/") != 0)
    {
        std::cerr << "Unable to chdir to '/'." << std::endl;
    }

    // make this the only running instance
    if (!lock_file_.lock())
    {
        return -4;
    }

    // register signals
    signal(SIGCHLD, SIG_IGN); /* ignore child */
    signal(SIGTSTP, SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, callback); /* catch hangup signal */
    signal(SIGTERM, callback); /* catch kill signal */
    signal(SIGINT, callback); /* catch kill signal */

    return 0;
}


bool DaemonClass::stop_daemon()
{
    return lock_file_.unlock();
}


bool DaemonClass::undaemonize()
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


bool DaemonClass::is_daemonized()
{
    return false;
}


int DaemonClass::get_daemon_pid()
{
    if (lock_file_.file_exists())
    {
        std::string content = lock_file_.get_file_content();

        return std::stoi(content);
    }

    return -1;
}


void DaemonClass::open_port()
{

    listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr_, '0', sizeof(serv_addr_));
    memset(sendBuff_, '0', sizeof(sendBuff_));

    serv_addr_.sin_family = AF_INET;
    serv_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr_.sin_port = htons(5000);


    listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr_, '0', sizeof(serv_addr_));
    memset(sendBuff_, '0', sizeof(sendBuff_));

    serv_addr_.sin_family = AF_INET;
    serv_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr_.sin_port = htons(5000);
}


void DaemonClass::close_port() {}
