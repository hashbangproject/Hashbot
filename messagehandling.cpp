// Copyright (C) 2013 Project Hashbang
// Created by Sultan Qasim Khan

#include <stellarino.h>
#include <messaging.h>
#include <gait.h>
#include <hashbot.h>

extern StagSystem g_stag;

void handlePing(uint8_t msgLength, const uint8_t *msg);
void handleMode(uint8_t msgLength, const uint8_t *msg);
void handleMove(uint8_t msgLength, const uint8_t *msg);
void handleMoveJoint(uint8_t msgLength, const uint8_t *msg);
void handleMoveJoints(uint8_t msgLength, const uint8_t *msg);

static msgHandler_t handlers[] =
{
        NULL,               // CONFIRMATION_MSG
        handlePing,         // PING_MSG
        handleMode,         // MODE_MSG
        NULL,               // ERROR_MSG
        handleMove,         // MOVE_MSG
        handleMoveJoint,    // MOVE_JOINT_MSG
        handleMoveJoints,   // MOVE_JOINTS_MSG
        NULL,               // BODY_SENSOR_MSG
        NULL,               // SYSTEM_STATUS_MSG
        NULL,               // TARGET_HIT_MSG
        NULL                // DEATH_MSG
};

void handleMessage(MessageType msgType, uint8_t msgLength, const uint8_t *msg)
{
    // Global interrupt disable
    ROM_IntMasterDisable();

    if (msgType > sizeof(handlers)) sendError(UNKNOWN_MESSAGE_ERROR);

    if (handlers[msgType]) handlers[msgType](msgLength, msg);
    else sendError(UNEXPECTED_MESSAGE_ERROR);

    // Global interrupt enable
    ROM_IntMasterEnable();
}

void sendError(ErrorCode err)
{
    uint8_t body = (uint8_t)err;
    putMessage(ERROR_MSG, 1, 0, &body);
}

void handlePing(uint8_t msgLength, const uint8_t *msg)
{
    // nothing to do, we already sent a confirmation if requested
}

void handleMode(uint8_t msgLength, const uint8_t *msg)
{
    // TODO: Do something for this
}

struct MoveMessage
{
    float x;    // X velocity
    float y;    // Y velocity
    float z;    // Angular velocity about Z axis
};

void handleMove(uint8_t msgLength, const uint8_t *msg)
{
    MoveMessage *mm = (MoveMessage *)msg;
    g_stag.setSpeed(mm->x, mm->y);
    g_stag.setRotation(mm->z);
}

struct MoveJointMessage
{
    int32_t leg;
    float angle;
};

void handleMoveJoint(uint8_t msgLength, const uint8_t *msg)
{
    MoveJointMessage *mjm = (MoveJointMessage *)msg;
    servoWrite(legPins[mjm->leg], servoPulseOffset[mjm->leg] + mjm->angle*angleToPulseFactor);
}

struct MoveJointsMessage
{
    // Each leg has 3 servos (A, B and C)
    float leg1A, leg1B, leg1C;
    float leg2A, leg2B, leg2C;
    float leg3A, leg3B, leg3C;
    float leg4A, leg4B, leg4C;
};

void handleMoveJoints(uint8_t msgLength, const uint8_t *msg)
{
    MoveJointsMessage *mjm = (MoveJointsMessage *)msg;
    servoWrite(legPins[0], servoPulseOffset[0] + mjm->leg1A*angleToPulseFactor);
    servoWrite(legPins[1], servoPulseOffset[1] + mjm->leg1B*angleToPulseFactor);
    servoWrite(legPins[2], servoPulseOffset[2] + mjm->leg1C*angleToPulseFactor);
    servoWrite(legPins[3], servoPulseOffset[3] + mjm->leg2A*angleToPulseFactor);
    servoWrite(legPins[4], servoPulseOffset[4] + mjm->leg2B*angleToPulseFactor);
    servoWrite(legPins[5], servoPulseOffset[5] + mjm->leg2C*angleToPulseFactor);
    servoWrite(legPins[6], servoPulseOffset[6] + mjm->leg3A*angleToPulseFactor);
    servoWrite(legPins[7], servoPulseOffset[7] + mjm->leg3B*angleToPulseFactor);
    servoWrite(legPins[8], servoPulseOffset[8] + mjm->leg3C*angleToPulseFactor);
    servoWrite(legPins[9], servoPulseOffset[9] + mjm->leg4A*angleToPulseFactor);
    servoWrite(legPins[10], servoPulseOffset[10] + mjm->leg4B*angleToPulseFactor);
    servoWrite(legPins[11], servoPulseOffset[11] + mjm->leg4C*angleToPulseFactor);
}
