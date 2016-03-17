#include <env.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ipc/Server.h"

int workProcess() {
    leveldb_daemon::ipc::Server server;
    server.work();
    server.destroy();
    return 0;
}


int monitorProcess() {
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
            retval = workProcess();
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

int main() {
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

        auto status = monitorProcess();

        return status;
    } else return 0;
}