#pragma once

#include <ktypes.h>
#include <sequencer.hpp>
#include <types.hpp>

class IPC
{
    static Sequencer<int> ids;
public:
    static int SendMessage(pid_t dst, int number, int flags, void *payload, size_t payloadSize);
    static int AckMessage(pid_t source, int id);
    static int GetMessage(ipcMessage *msg, int timeout);
};
