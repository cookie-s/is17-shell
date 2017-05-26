#include <stdlib.h>
#include <signal.h>

#include "signals.h"

void ignore_stop_signals(void) {
    sigset_t sigmask;
    sigemptyset(&sigmask);

    struct sigaction sig = {
        .sa_mask  = sigmask,
        .sa_flags = SA_RESTART,
    };

    // SIGINT
    sig.sa_handler = SIG_IGN;
    sigaction(SIGINT, &sig, NULL);

    // SIGTERM
    sig.sa_handler = SIG_IGN;
    sigaction(SIGTERM, &sig, NULL);

    // SIGTSTP
    sig.sa_handler = SIG_IGN;
    sigaction(SIGTSTP, &sig, NULL);

    // SIGTTIN
    sig.sa_handler = SIG_IGN;
    sigaction(SIGTTIN, &sig, NULL);

    // SIGTTOU
    sig.sa_handler = SIG_IGN;
    sigaction(SIGTTOU, &sig, NULL);
}

void dfl_stop_signals(void) {
    sigset_t sigmask;
    sigemptyset(&sigmask);

    struct sigaction sig = {
        .sa_mask  = sigmask,
        .sa_flags = SA_RESTART,
    };

    // SIGINT
    sig.sa_handler = SIG_DFL;
    sigaction(SIGINT, &sig, NULL);

    // SIGTERM
    sig.sa_handler = SIG_DFL;
    sigaction(SIGTERM, &sig, NULL);

    // SIGTSTP
    sig.sa_handler = SIG_DFL;
    sigaction(SIGTSTP, &sig, NULL);

    // SIGTTIN
    sig.sa_handler = SIG_DFL;
    sigaction(SIGTTIN, &sig, NULL);

    // SIGTTOU
    sig.sa_handler = SIG_DFL;
    sigaction(SIGTTOU, &sig, NULL);
}

void handle_sigchld(void(*handler)(int)) {
    sigset_t sigmask;
    sigemptyset(&sigmask);

    struct sigaction sig = {
        .sa_mask  = sigmask,
        .sa_flags = SA_RESTART,
    };

    // SIGCHLD
    sig.sa_handler = handler;
    sigaction(SIGCHLD, &sig, NULL);
}

void block_sigchld(void) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGCHLD);
    sigprocmask(SIG_BLOCK, &sigset, NULL);
}

void unblock_sigchld(void) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGCHLD);
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);
}
