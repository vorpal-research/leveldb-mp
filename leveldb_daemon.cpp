#include <env.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ipc/Server.h"

const std::string DAEMON_FILE_PATH = "/tmp/leveldb_daemon_started";

int workProcess(const std::string& db_name, const std::string& socket_name) {
    leveldb_daemon::ipc::Server server(db_name, socket_name);
    server.work();
    server.destroy();
    return 0;
}


int monitorProcess(const std::string& db_name, const std::string& socket_name) {
    int pid, retval;
    sigset_t sigset;
    siginfo_t siginfo;
    leveldb_daemon::logging::Logger log;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGQUIT);        //process stopped by user
    sigaddset(&sigset, SIGINT);         //process stopped in terminal
    sigaddset(&sigset, SIGTERM);        //process end request
    sigaddset(&sigset, SIGCHLD);        //child process status changed

    sigprocmask(SIG_BLOCK, &sigset, NULL);

    while(true) {

        pid = fork();

        if (pid == -1) {
            log.print("[MONITOR] Fork failed \n");
        } else if (not pid) {
            retval = workProcess(db_name, socket_name);

            if (std::remove(DAEMON_FILE_PATH.c_str()) != 0) {
                log.print("error while deleting daemon file");
            }

            exit(retval);
        } else {
            sigwaitinfo(&sigset, &siginfo);

            if (siginfo.si_signo == SIGCHLD) {
                wait(&retval);
            } else {
                kill(pid, SIGKILL);
                retval = 0;

                if (std::remove(DAEMON_FILE_PATH.c_str()) != 0) {
                    log.print("error while deleting daemon file");
                }

                break;
            }
        }
    }
    return retval;
}

int main(int argc, char** argv) {
    if (argc < 3) return -1;
    std::string db_name(argv[1]), socket_name(argv[2]);

    auto pid = fork();

    if (pid == -1) {
        return -1;
    } else if (not pid) {
        umask(0);
        setsid();

        chdir("/");

        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        auto status = monitorProcess(db_name, socket_name);

        return status;
    } else {
        std::ofstream file;
        file.open(DAEMON_FILE_PATH);
        file << pid << std::endl;
        file.close();
        return 0;
    }
}