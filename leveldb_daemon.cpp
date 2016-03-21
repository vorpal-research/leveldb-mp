#include <env.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ipc/Server.h"

int workProcess(int argc, char** argv) {
    std::string db_name(argv[0]);
    std::string socket_name(argv[1]);
    leveldb_daemon::ipc::Server server(db_name, socket_name);
    server.work();
    server.destroy();
    return 0;
}


int monitorProcess(int argc, char** argv) {
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
            retval = workProcess(argc, argv);
            exit(retval);
        } else {
            sigwaitinfo(&sigset, &siginfo);

            if (siginfo.si_signo == SIGCHLD) {
                wait(&retval);
            } else {
                kill(pid, SIGKILL);
                retval = 0;
                break;
            }
        }
    }
    return retval;
}

int main(int argc, char** argv) {
    if (argc < 2) return -1;

    auto pid = fork();

    if (pid == -1) {
        return -1;
    } else if (!pid) {
        umask(0);
        setsid();

        chdir("/");

        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        auto status = monitorProcess(argc, argv);

        return status;
    } else return 0;
}