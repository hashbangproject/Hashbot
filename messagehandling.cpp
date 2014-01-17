// Copyright (C) 2013 Project Hashbang
// Created by Sultan Qasim Khan

#include <stellarino.h>
#include <messaging.h>
#include <gait.h>

extern StagSystem g_stag;

void handleMessage(MessageType msgType, uint8_t msgLength, const uint8_t *msg)
{
    // Global interrupt disable
    ROM_IntMasterDisable();

    float a, b;

    switch (msgType)
    {
    case PING_MSG:
        putMessage(PONG_MSG, 0, NULL);
        break;
    case PONG_MSG:
        sendError(UNEXPECTED_MESSAGE_ERROR);
        break;
    case SET_SPEED_MSG:
        a = *((float *)msg);
        b = *((float *)msg + 1);
        g_stag.setSpeed(a, b);
        confirmMessage(SET_SPEED_MSG);
        break;
    default:
        sendError(UNKNOWN_MESSAGE_ERROR);
        break;
    }

    // Global interrupt enable
    ROM_IntMasterEnable();
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
