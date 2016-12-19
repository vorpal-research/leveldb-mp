#include <env.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "Config.h"
#include "ipc/Server.h"
#include "util/Util.h"

std::string daemon_file = leveldb_mp::config::DAEMON_FILE_PATH;

int workProcess(const std::string& socket_name) {
    using namespace leveldb_mp;
    std::string name = config::OUTPUT_FILE_PATH + socket_name;
    ipc::Server server(name, name + config::SOCKET_POSTFIX);
    server.work();
    server.destroy();
    return 0;
}


int monitorProcess(const std::string& socket_name) {
    int pid, retval;
    sigset_t sigset;
    siginfo_t siginfo;

    using namespace leveldb_mp::logging;
    Logger log;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGQUIT);        //process stopped by user
    sigaddset(&sigset, SIGINT);         //process stopped in terminal
    sigaddset(&sigset, SIGTERM);        //process end request
    sigaddset(&sigset, SIGCHLD);        //child process status changed

    sigprocmask(SIG_BLOCK, &sigset, NULL);

    while(true) {

        pid = fork();

        if (pid == -1) {
            log << "[MONITOR] Fork failed" << endl;
        } else if (not pid) {
            retval = workProcess(socket_name);

            if (std::remove(daemon_file.c_str()) != 0) {
                log << "error while deleting daemon file" << endl;
            }

            exit(retval);
        } else {
            sigwaitinfo(&sigset, &siginfo);

            if (siginfo.si_signo == SIGCHLD) {
                wait(&retval);
            } else {
                kill(pid, SIGKILL);
                retval = 0;

                if (std::remove(daemon_file.c_str()) != 0) {
                    log << "error while deleting daemon file" << endl;
                }

                break;
            }
        }
    }
    return retval;
}

int main(int argc, char** argv) {
    if (argc < 2) return -1;
    std::string socket_name(argv[1]);
    daemon_file += socket_name;

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

        return monitorProcess(socket_name);
    } else {
        std::ofstream file(daemon_file);
        file << pid << std::endl;
        file << socket_name << std::endl;
        return 0;
    }
}