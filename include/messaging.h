// Copyright (C) 2013 Project Hashbang
// Created by Sultan Qasim Khan

#ifndef MESSAGING_H
#define MESSAGING_H

#include <stdint.h>

typedef enum
{
    PING_MSG,           // The recipient will reply with a PONG_MSG
    PONG_MSG,           // Reply to a PING_MSG
    CONFIRMATION_MSG,   // Sent by the recipient of a message to confirm receipt
                        // The message body should be the message type of the received message
                        // This will only be sent for safety critical messages

    // Safety Critical Messages
    // Thus far, all these messages are sent from the host
    ENABLE_MSG,         // Message to enable the servos and air dart at a software level
    DISABLE_MSG,        // Message to disable the servos and air dart at a software level
    SINGLE_FIRE_MSG,    // Fire a single dart
    START_CONT_FIRE_MSG,// Start firing darts continuously
    STOP_CONT_FIRE_MSG, // Stop firing darts continuously
    ERROR_MSG,          // An error occurred (body will hold error code)

    // Non-Safety Critical Messages
    SET_SPEED_MSG,      // Sent by the host to indicate XY walking speed (based off joystick)
    SET_HEIGHT_MSG,     // Sent by the host to indicate the walking height for the robot
    SET_TURRET_MSG,     // Sent by the host to set the turret rotation speed/direction
    BATTERY_MSG,        // Sent automatically by the robot to the host periodically
    LEG_POSITION_MSG,   // Sent automatically by the robot to the host every 50 ms
    SENSOR_READING_MSG, // Sent automatically by the robot to the host periodically
    TARGET_HIT_MSG      // Sent automatically by the robot to the host when hit
} MessageType;

// Error codes for error messages
typedef enum
{
    UNKNOWN_MESSAGE_ERROR,
    UNEXPECTED_MESSAGE_ERROR,
    TIMEOUT_ERROR,
    CRC_ERROR
} ErrorCode;

void enableMessaging(void);

void putMessage(MessageType msgType, uint8_t msgLength, const uint8_t *msg);
int getMessage(MessageType *msgType, uint8_t *msgLength, uint8_t *msg);

void handleMessage(MessageType msgType, uint8_t msgLength, const uint8_t *msg);
void confirmMessage(MessageType msgType);
void sendError(ErrorCode err);

#endif
