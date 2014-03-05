// Copyright (C) 2014 Project Hashbang
// Created by Sultan Qasim Khan

#ifndef HASHBOT_H
#define HASHBOT_H

static const unsigned char legPins[12] =
{
    PB5, PB0, PD0,  // Leg 1
    PB1, PD1, PD3,  // Leg 2
    PB2, PB3, PC4,  // Leg 3
    PC5, PB7, PB6   // Leg 4
};

static const float angleToPulseFactor = 752.0f;

// Servo Pulse ~= servoPulseOffset + angleInRadians*angleToPulseFactor
static const short servoPulseOffset[12] =
{
    1350, 930, 410,     // Leg 1
    1350, 930, 410,     // Leg 2
    1350, 2060, 2590,   // Leg 3
    1350, 2060, 2590    // Leg 4
};

#endif /* HASHBOT_H */
