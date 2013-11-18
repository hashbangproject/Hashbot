// Copyright (C) 2013 Project Hashbang
// Created by Andrew Yang

#include <gait.h>
#include <stdint.h>
#include <math.h> //should be unnecessary if using the dsp library
#include <stellarino.h> // for micros()

//arm function prototypes
static inline void arm_add_f32(float *pSrcA, float *pSrcB, float *pDst, uint32_t blockSize){ for(int n=0; n<blockSize; n++) pDst[n] = pSrcA[n] + pSrcB[n]; }
static inline void arm_scale_f32(float* src, float s, float* dst, int size){ for(int n=0; n<size; n++) dst[n] = s*src[n]; }
static inline float arm_cos_f32(float x) { return cos(x); }
static inline float arm_sin_f32(float x) { return sin(x); }
static inline void arm_sqrt_f32(float in, float *pOut) { *pOut = sqrt(in); }

vector::vector() : x(p[0]), y(p[1]), z(p[2]) {}

vector::vector(const vector &v) : x(p[0]), y(p[1]), z(p[2])
{
    operator =(v);
}

vector::vector(float x, float y, float z) : x(p[0]), y(p[1]), z(p[2])
{
    p[0] = x; p[1] = y; p[2] = z;
}

vector vector::operator+(vector other)
{
    vector ret;
    arm_add_f32(p, other.p, ret.p, 3);
    return ret;
}

vector vector::operator-() //unary negate
{
    vector ret;
    arm_scale_f32(p, -1, p, 3);
    return ret;
}

vector vector::operator*(float x)
{
    vector ret;
    arm_scale_f32(p, x, ret.p, 3);
    return ret;
}
vector & vector::operator*=(float x)
{
    arm_scale_f32(p, x, p, 3);
    return *this;
}

vector & vector::operator=(const vector &other) //copy
{
    arm_scale_f32(const_cast<float*>(other.p), 1, p, 3);
    return *this;
}
void vector::rotate(float theta, vector center)
{
    float cosTheta = arm_cos_f32(theta);
    float sinTheta = arm_sin_f32(theta);

    float tx = x - center.x, ty = y - center.y;
    x = center.x + tx*cosTheta - ty*sinTheta;
    y = center.y + tx*sinTheta + ty*cosTheta;
}



const float StagSystem::liftHeight = 1;

StagSystem::StagSystem():
    tMicros(0),
    periodMicros(1000000),
    xSpeed(0.f),
    ySpeed(0.f),
    rotation(0.f),
    zOffset(100.f)
{
    // 0--1     x
    // |  |     |_ y
    // 3--2

    // the below numbers should probably be member variables passed in as constructor parameters
    // they should also probably be given in units that make sense
    vector cog(0.f, 0.f, -100.f);
    vector dimensions(180.f, 140.f, -cog.z);
    float lPelvis = 20.f;
    float lUpperLeg = 70.f;
    float lLowerLeg = 60.f;
    legs[0] = Leg(+dimensions.x/2.f, -dimensions.y/2.f, dimensions.z, 0.00f, 0.8f, lPelvis, lUpperLeg, lLowerLeg, true, true);
    legs[1] = Leg(+dimensions.x/2.f, +dimensions.y/2.f, dimensions.z, 0.50f, 0.8f, lPelvis, lUpperLeg, lLowerLeg, true, false);
    legs[2] = Leg(-dimensions.x/2.f, +dimensions.y/2.f, dimensions.z, 0.75f, 0.8f, lPelvis, lUpperLeg, lLowerLeg, false, false);
    legs[3] = Leg(-dimensions.x/2.f, -dimensions.y/2.f, dimensions.z, 0.25f, 0.8f, lPelvis, lUpperLeg, lLowerLeg, false, true);
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

void StagSystem::moveLegs()
{
    unsigned long dTMicros = micros() - tMicros;
    tMicros += dTMicros;

    vector position(xSpeed, ySpeed, zOffset); // TODO: scale
    float theta = rotation; // TODO: multiply by some constant
    float phase = (float)tMicros/periodMicros; //TODO: pause phase and dPhase when not moving
    float dPhase = (float)dTMicros/periodMicros;

    for(int i=0; i<4; i++)
    {
        legs[i].moveLeg(position, theta, phase, dPhase);
    }
}

void StagSystem::getLeg(int leg, float *pAngle1, float *pAngle2, float *pAngle3)
{

    this->legs[leg].getAngles(pAngle1, pAngle2, pAngle3);
}

StagSystem::Leg::Leg(float x, float y, float z, float offset, float duty, float lPelvis, float lUpperLeg, float lLowerLeg, bool isFront, bool isLeft) :
        hip_cog(x,y,0),
        foot_hip(0,0,z),
        offset(offset),
        duty(duty),
        lPelvis(lPelvis),
        lUpperLeg(lUpperLeg),
        lLowerLeg(lLowerLeg),
        isFront(isFront),
        isLeft(isLeft)
{}

void StagSystem::Leg::moveLeg(vector position, float theta, float phase, float dPhase)
{
    //offset StagSystem phase to our own phase
    phase += offset;
    phase -= (int)phase;

    if(phase < duty)
    {
        // Foot on ground
        // Move away from direction vector
        foot_hip.z = position.z;
        foot_hip.x -= position.x * 2 * (dPhase/duty);
        foot_hip.y -= position.y * 2 * (dPhase/duty);
        // Rotate
        if(theta != 0.f)
        {
            foot_hip.rotate(-theta*(dPhase/duty), -hip_cog);
        }
    }
    else
    {
        // Foot in air
        vector target = position;
        // Rotate
        if(theta != 0.f)
        {
            target.rotate(theta*(1.f-duty), -hip_cog);
        }
        // Move toward target
        if(1 - phase > dPhase)
        {
            foot_hip.x += (foot_hip.x - target.x) * dPhase/(1-phase);
            foot_hip.y += (foot_hip.y - target.y) * dPhase/(1-phase);
            foot_hip.z = position.z - StagSystem::liftHeight;
        }
        else
        {
            //last iteration
            foot_hip = position;
        }
    }
}

//is there an arm function for doing this?
inline float sqr(float x){ return x*x; }
void StagSystem::Leg::getAngles(float *pAngle1, float *pAngle2, float *pAngle3)
{
    float sign1 = isLeft ? -1.f : 1.f;
    float sign2 = isFront ? 1.f : -1.f;
    float sqrtResult;
    const float pi = 3.14159265358f;

    //the returned angles could be totally wrong. I'm not sure. haven't done any testing.
    //TODO: error check the arm_sqrt_f32 return value. maybe. if we're not lazy.
    arm_sqrt_f32(sqr(foot_hip.y)+sqr(foot_hip.z), &sqrtResult);
    *pAngle1 = atan2(foot_hip.z,sign1*foot_hip.y) - acos(lPelvis/sqrtResult);
    vector pelvis_hip
    (
        0.f,
        sign1*lPelvis*cos(*pAngle1),
        lPelvis*sin(*pAngle1)
    );

    float fpDeltaSqr = sqr(foot_hip.x-pelvis_hip.x) + sqr(foot_hip.y-pelvis_hip.y) + sqr(foot_hip.z-pelvis_hip.z);
    arm_sqrt_f32(fpDeltaSqr, &sqrtResult);
    *pAngle2 = atan2(foot_hip.x, (foot_hip.z-pelvis_hip.z)/cos(*pAngle1)) + sign2*acos((sqr(lUpperLeg) + fpDeltaSqr - sqr(lLowerLeg))/(2.f * lUpperLeg * sqrtResult));
    *pAngle3 = sign2*pi - sign2*acos((sqr(lUpperLeg) + sqr(lLowerLeg) - fpDeltaSqr) / (2.f * lUpperLeg * lLowerLeg));
}
