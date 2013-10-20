// Copyright (C) 2013 Project Hashbang
// Created by Sultan Qasim Khan

#ifndef MESSAGING_H
#define MESSAGING_H

void putMessage(uint8_t msgType, uint8_t msgLength, const uint8_t *msg);
int getMessage(uint8_t *msgType, uint8_t *msgLength, uint8_t *msg);

#endif
