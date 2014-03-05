// Copyright (C) 2013 Project Hashbang
// Created by Sultan Qasim Khan

#ifndef MESSAGING_H
#define MESSAGING_H

#include <stdint.h>

#define MAX_MESSAGE_LENGTH 512

typedef struct MsgHeaderStruct
{
    uint16_t crc;
    uint16_t type;
    uint16_t flags;
    uint16_t length;
    uint32_t id;
} MsgHeader;

// Message id is incremented with every message sent
// Even message ids are from the controlling computer, odd message ids are from the robot

#define CONFIRMATION_NEEDED_FLAG 0x01

typedef enum
{
    CONFIRMATION_MSG = 0,   // Sent by the recipient of a message to confirm receipt
                            // The message body should be the message id of the received message
                            // This will only be sent for safety critical messages

    // Safety Critical Messages (will request a confirmation)
    PING_MSG = 1,           // The sender just wants a confirmation
    MODE_MSG = 2,           // Message will be a RobotModeStruct
    ERROR_MSG = 3,          // An error occurred (body will hold error code)

    // Other messages
    MOVE_MSG = 4,           // Contains a MoveMessage struct
    MOVE_JOINT_MSG = 5,     // Contains a MoveJointMessage struct
    MOVE_JOINTS_MSG = 6,    // Contains a MoveJointsMessage struct
    BODY_SENSOR_MSG = 7,    // Contains high frequency sensor readings
    SYSTEM_STATUS_MSG = 8,  // Contains battery status and other low frequency status info

    // Push notifications
    TARGET_HIT_MSG = 9,     // Sent automatically by the robot to the host when hit
    DEATH_MSG = 10,         // Sent automatically when the robot runs out of health

    // Geophilosophical messages
    OBJECTIVIST_MSG,    // The robot will try to think rationally
    POSITIVIST_MSG,     // The robot will follow the scientific method
    HUMANIST_MSG,       // The robot will try to relate to the feelings of those it is attacking
    FEMINIST_MSG,       // The robot will divorce her owner and take 2/3 of his money
    COMMUNIST_MSG,      // The robot will push for it to be the most equal of all robots
    POSTMODERNIST_MSG,  // The robot will stop following its master and will start criticising her/him

    // Possible Messages
    SET_HEIGHT_MSG,     // Sent by the host to indicate the walking height for the robot
    SET_TURRET_MSG      // Sent by the host to set the turret rotation speed/direction
} MessageType;

// Error codes for error messages
typedef enum
{
    UNKNOWN_MESSAGE_ERROR,
    UNEXPECTED_MESSAGE_ERROR,
    TIMEOUT_ERROR,
    CRC_ERROR
} ErrorCode;

typedef void (*msgHandler_t)(uint8_t msgLength, const uint8_t *msg);

void enableMessaging(void);

void putMessage(MessageType msgType, uint16_t msgLength, uint16_t flags, const uint8_t *msg);
int getMessage(MessageType *msgType, uint16_t *msgLength, uint16_t *flags, uint8_t *msg);

void handleMessage(MessageType msgType, uint8_t msgLength, const uint8_t *msg);
void sendError(ErrorCode err);

#endif
