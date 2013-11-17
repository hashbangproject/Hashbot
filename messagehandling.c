// Copyright (C) 2013 Project Hashbang
// Created by Sultan Qasim Khan

#include <messaging.h>

void handleMessage(MessageType msgType, uint8_t msgLength, const uint8_t *msg)
{
    // TODO: Global interrupt disable

    switch (msgType)
    {
    case PING_MSG:
        putMessage(PONG_MSG, 0, NULL);
        break;
    case PONG_MSG:
        sendError(UNEXPECTED_MESSAGE_ERROR);
        break;
    default:
        sendError(UNKNOWN_MESSAGE_ERROR);
        break;
    }

    // TODO: Global interrupt enable
}

void confirmMessage(MessageType msgType)
{
    uint8_t body = (uint8_t)msgType;
    putMessage(CONFIRMATION_MSG, 1, &body);
}

void sendError(ErrorCode err)
{
    uint8_t body = (uint8_t)err;
    putMessage(ERROR_MSG, 1, &body);
}
