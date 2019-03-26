#pragma once

#define MSG_PAYLOAD_SIZE            112

#define MSG_FLAG_NONE               0
#define MSG_FLAG_ACK_REQ            1

#define MSG_NULL                    0
#define MSG_ACK                     1
#define MSG_NACK                    2
#define MSG_PING                    3
#define MSG_PONG                    4
#define MSG_QUIT                    5

#define MSG_ACQUIRE_KEYBOARD        100
#define MSG_RELEASE_KEYBOARD        101
#define MSG_KEYBOARD_EVENT          102
#define MSG_ACQUIRE_MOUSE           103
#define MSG_RELEASE_MOUSE           104
#define MSG_MOUSE_EVENT             105

#define MSG_RPC_REQUEST             200
#define MSG_RPC_RESPONSE            201
#define MSG_RPC_FIND_SERVER         202
#define MSG_RPC_FIND_SERVER_RESP    203

#include <inputdevtypes.h>

#define MSG_RPC_RESP_PAYLOAD_SIZE (MSG_PAYLOAD_SIZE - sizeof(int))

struct ipcRPCResponse
{
    int RequestMessageID;
    unsigned char Results[MSG_RPC_RESP_PAYLOAD_SIZE];
};

struct ipcMessage
{
    int Number, Flags, ID, Source;
    union
    {
        unsigned char Data[MSG_PAYLOAD_SIZE];
        struct inpKeyboardEvent Keyboard;
        struct inpMouseEvent Mouse;
        struct ipcRPCResponse RPCResponse;
    };
};
