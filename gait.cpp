// Copyright (C) 2013 Project Hashbang
// Created by Andrew Yang

#include "gait.h"
#include <stdint.h>
#include <math.h>
#include <stellarino.h> // for micros()

// helper functions
static inline float interpolate(float i1, float i2, float o1, float o2, float x)
{
    return o1 + (o2-o1)*(x-i1)/(i2-i1);
}

static inline float mix(float o1, float o2, float x)
{
    return o1 + (o2-o1)*x;
}

static inline float sqr(float x)
{
    return x*x;
}

// implementation of vector

vector vector::operator+(const vector &other) const
{
    return vector(x+other.x, y+other.y, z+other.z);
}

vector vector::operator-() const //unary negate
{
    return vector(-x, -y, -z);
}

vector vector::operator-(const vector &other) const
{
    return vector(x-other.x, y-other.y, z-other.z);
}

vector vector::operator*(float x) const
{
    return vector(this->x*x, y*x, z*x);
}

vector & vector::operator*=(float x)
{
    this->x *= x;
    this->y *= x;
    this->z *= x;
    return *this;
}

vector & vector::operator+=(const vector &other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

vector & vector::operator-=(const vector &other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

void vector::rotate(float theta, vector center)
{
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);

    float tx = x - center.x, ty = y - center.y;
    x = center.x + tx*cosTheta - ty*sinTheta;
    y = center.y + tx*sinTheta + ty*cosTheta;
}


// implementation of gait classes

const float StagSystem::liftHeight = 10;
const float StagSystem::gaitOffsets[GaitType::GAIT_TYPES][4] = 
{
    {0.00f, 0.50f, 0.75f, 0.25f},
    {0.00f, 0.50f, 1.00f, 0.50f},
    {0.00f, 0.30f, 1.00f, 0.70f},
    {0.00f, 0.10f, 0.60f, 0.50f},
    {0.00f, 0.00f, 0.50f, 0.50f},
    {0.00f, 0.00f, 0.00f, 0.00f},
};
const float StagSystem::gaitDuty[GaitType::GAIT_TYPES] = 
{
    0.80f,
    0.75f,
    0.75f,
    0.65f,
    0.55f,
    0.55f
};
const float StagSystem::gaitShake[GaitType::GAIT_TYPES] = 
{
    3.f,
    1.f,
    0.5f,
    0.f,
    0.f,
    0.f
};
const unsigned long StagSystem::gaitPeriod[GaitType::GAIT_TYPES] = 
{
    2000000,
    1000000,
    666666,
    500000,
    400000,
    333333
};

StagSystem::StagSystem():
    tMicros(0),
    periodMicros(2000000),
    xSpeed(0.f),
    ySpeed(0.f),
    rotation(0.f),
    zOffset(100.f),
    gaitType(CRAWL),
    shakeFactor(1.f),
    duty(gaitDuty[0]),
    tMicros(0),
    periodMicros(2000000),

    phase(0.f),
    gaitTransition(CRAWL)
{
    // 0--1     x
    // |  |     |_ y
    // 3--2

    // the below numbers should probably be member variables passed in as constructor parameters
    // they should also probably be given in units that make sense
    vector cog(0.f, 0.f, -90.f);
    vector dimensions(155.f, 20.f, -cog.z);
    float lPelvis = 20.f;
    float lUpperLeg = 70.f;
    float lLowerLeg = 70.f;

    legs[0] = new Leg(+dimensions.x/2.f, -dimensions.y/2.f, dimensions.z, gaitOffsets[0][0], duty, lPelvis, lUpperLeg, lLowerLeg, true, true);
    legs[1] = new Leg(+dimensions.x/2.f, +dimensions.y/2.f, dimensions.z, gaitOffsets[0][1], duty, lPelvis, lUpperLeg, lLowerLeg, true, false);
    legs[2] = new Leg(-dimensions.x/2.f, +dimensions.y/2.f, dimensions.z, gaitOffsets[0][2], duty, lPelvis, lUpperLeg, lLowerLeg, false, false);
    legs[3] = new Leg(-dimensions.x/2.f, -dimensions.y/2.f, dimensions.z, gaitOffsets[0][3], duty, lPelvis, lUpperLeg, lLowerLeg, false, true);
}

void StagSystem::setSpeed(float x, float y)
{
    xSpeed = x;
    ySpeed = y;
}

void StagSystem::setRotation(float rot)
{
    rotation = rot;
}

void StagSystem::setHeight(float zOff)
{
    zOffset = zOff;
}

void StagSystem::shiftGear(bool faster)
{
    if(faster && gaitType < GAIT_TYPES-1)
        gaitType = (GaitType)(gaitType + 1);
    if(!faster && gaitType > 0)
        gaitType = (GaitType)(gaitType - 1);
}

void StagSystem::updateGait(float updateSpeed)
{
    if(gaitTransition > gaitType) gaitTransition -= updateSpeed;
    if(gaitTransition < gaitType) gaitTransition += updateSpeed;
    if(abs(gaitTransition-gaitType) < updateSpeed) gaitTransition = (float)gaitType;

    float interpolationFactor = gaitTransition - (int)gaitTransition;
    shakeFactor = mix(gaitShake[(int)gaitTransition],
                      gaitShake[(int)gaitTransition+1],
                      interpolationFactor);
    periodMicros = mix(gaitPeriod[(int)gaitTransition],
                       gaitPeriod[(int)gaitTransition+1],
                       interpolationFactor);
    duty = mix(gaitDuty[(int)gaitTransition],
                       gaitDuty[(int)gaitTransition+1],
                       interpolationFactor);
    for(int i=0; i<4; i++)
    {
        legs[i]->offset = mix(gaitOffsets[(int)gaitTransition][i],
                             gaitOffsets[(int)gaitTransition+1][i],
                             interpolationFactor);
    }
}

void StagSystem::moveLegs() 
{
    // timing
    unsigned long dTMicros = micros() - tMicros;
    tMicros += dTMicros;

    // update gait parameters
    updateGait(dTMicros/1000000.f);

    vector position(xSpeed, ySpeed, zOffset); // TODO: scale
    float theta = rotation; // TODO: multiply by some constant
    float dPhase = (float)dTMicros/periodMicros;
    phase += dPhase;
    phase = phase - (int)phase;

    // stop if we're not actually moving
    if(xSpeed == 0.f && ySpeed == 0.f && theta == 0.f)
    {
        for(int i=0; i<4; i++)
        {
            if(abs(phase - legs[i]->offset) <= 2.f*dPhase){
                phase = legs[i]->offset;
                break;
            }
        }
    }

    // accumulate hip shake factor for each leg
    cog_offset.x = cog_offset.y = 0.f;
    float weight = 0.f;
    for(int i=0; i<4; i++)
        legs[i]->calculateHipShake(!(xSpeed == 0.f && ySpeed == 0.f && theta == 0.f), phase, &cog_offset, &weight);

    cog_offset *= shakeFactor/weight;

    // move each leg
    for(int i=0; i<4; i++)
        legs[i]->moveLeg(position, theta, phase, dPhase, cog_offset);
}

void StagSystem::getLeg(int leg, float *pAngle1, float *pAngle2, float *pAngle3)
{
    this->legs[leg]->getAngles(pAngle1, pAngle2, pAngle3);
}

StagSystem::Leg::Leg(float x, float y, float z, float offset, float &duty, float lPelvis, float lUpperLeg, float lLowerLeg, bool isFront, bool isLeft) :
        hip_cog(x,y,0),
        foot_hip(0,(!isLeft-isLeft)*lPelvis,z),
        offset(offset),
        duty(duty),
        lPelvis(lPelvis),
        lUpperLeg(lUpperLeg),
        lLowerLeg(lLowerLeg),
        isFront(isFront),
        isLeft(isLeft)
{}

void StagSystem::Leg::calculateHipShake(float magnitude, float phase, vector *accumulate, float *weight)
{
    //offset StagSystem phase to our own phase
    phase += offset;
    phase -= (int)phase;

    float hipShake = 0.f;
    if(phase < duty*0.6) // leg on ground
        hipShake = interpolate(0, duty*0.6, 2.f, 1.f, phase);
    else if(phase < duty) // leg about to lift (get away from there!)
        hipShake = interpolate(duty*0.6, duty, 1.f, -0.5f, phase);
    else if(phase < 0.7+0.3*duty) // leg in air
        hipShake = interpolate(duty, 0.7+0.3*duty, -0.5f, 0.2f, phase);
    else // leg about to land (get there quickly!)
        hipShake = interpolate(0.7+0.3*duty, 1.f, 0.2f, 2.f, phase);

    hipShake = mix(1.f, hipShake, magnitude);
    accumulate->x += hipShake * (hip_cog.x+foot_hip.x);
    accumulate->y += hipShake * (hip_cog.y+foot_hip.y);
    *weight += hipShake;
}

void StagSystem::Leg::moveLeg(const vector &position, float theta, float phase, float dPhase, const vector &cog_offset)
{
    //offset StagSystem phase to our own phase
    phase += offset;
    phase -= (int)phase;

    if(phase < duty)
    {
        // Foot on ground
        // Move away from direction vector
        foot_hip -= position * (2.f * (dPhase/duty));
        foot_hip.z = position.z;

        // Rotate
        if(theta != 0.f)
            foot_hip.rotate(-theta*(dPhase/duty), -hip_cog);
        
        // Compensate for cog_offset
        foot_hip -= cog_offset*dPhase;
    }
    else
    {
        // Foot in air
        vector target = position;
        target.y -= (isLeft-!isLeft)*lPelvis;
        if(phase < 0.5f+duty*0.5f) target.z -= StagSystem::liftHeight;

        // Rotate
        if(theta != 0.f)
            target.rotate(theta*(1.f-duty), -hip_cog);

        if(1 - phase > dPhase) // Move toward target
            foot_hip += (target - foot_hip) * (dPhase/(1.f-phase));
        else //last iteration
            foot_hip = target;
    }
}

void StagSystem::Leg::getAngles(float *pAngle1, float *pAngle2, float *pAngle3)
{
    float sign1 = isLeft ? -1.f : 1.f;
    float sign2 = isFront ? 1.f : -1.f;
    const float pi = 3.14159265358f;

    // Inverse Kinematics (i.e. black magic)
    *pAngle1 = atan2(foot_hip.y, foot_hip.z);
    vector pelvis_hip
    (
        0.f,
        lPelvis*sin(*pAngle1),
        lPelvis*cos(*pAngle1)
    );
    float fpZ = sqrt(sqr(foot_hip.y) + sqr(foot_hip.z)) - lPelvis;
    float fpDeltaSqr = sqr(foot_hip.x) + sqr(fpZ);
    *pAngle2 = atan2(fpZ, foot_hip.x) + sign2*acos((sqr(lUpperLeg)+fpDeltaSqr-sqr(lLowerLeg))/(2.f*lUpperLeg*sqrt(fpDeltaSqr)));
    *pAngle3 = sign2*(pi - acos((sqr(lUpperLeg)+sqr(lLowerLeg)-fpDeltaSqr)/(2.f*lUpperLeg*lLowerLeg)));
}
