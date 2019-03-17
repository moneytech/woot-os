#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define SIG_TID_SELF    (-1)
#define SIG_TID_ALL     0

void *sigGetHandler(int sig);
int sigSetHandler(int sig, void *handler);
int sigIsSignalEnabled(int sig);
int sigEnableSignal(int sig);
int sigDisableSignal(int sig);
int sigRaise(int tid, int sig);

#ifdef __cplusplus
}
#endif // __cplusplus
