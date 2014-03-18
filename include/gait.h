// Copyright (C) 2013 Project Hashbang
// Created by Andrew Yang

#ifndef GAIT_H
#define GAIT_H

// container for easy vector manipulation
struct vector
{
    vector() : x(0), y(0), z(0) {}
    vector(float x, float y, float z) : x(x), y(y), z(z) {}

    vector operator+(const vector &other) const;
    vector operator-(const vector &other) const;
    vector operator-() const; //unary negate
    vector operator*(float x) const;

    vector & operator*=(float x);
    vector & operator+=(const vector &other);
    vector & operator-=(const vector &other);

    void rotate(float theta, vector center);

    float x;
    float y;
    float z;
};

class StagSystem
{
    friend int main();
public:
    enum GaitType
    {
        CRAWL,
        TROT,
        CANTER,
        GALLOP,
        BOUND,
        PRONK,
        GAIT_TYPES
    };

    StagSystem();

    // Input parameters
    void setSpeed(float x, float y);
    void setRotation(float rot);
    void setHeight(float zOff);
    void shiftGear(bool faster);

    // Calculations
    void updateGait(float updateSpeed);
    void moveLegs();

    void getLeg(int leg, float *pAngle1, float *pAngle2, float *pAngle3);
private:
    class Leg
    {
        friend int main();
        friend class StagSystem;
    public:
        Leg(float x, float y, float z, float offset, float &duty, float lPelvis, float lUpperLeg, float lLowerLeg, bool isFront, bool isLeft);
        void calculateHipShake(float magnitude, float phase, vector *accumulate, float *weight);
        void moveLeg(const vector &position, float theta, float phase, float dPhase, const vector &cog_offset);
        //angles returned in radians
        void getAngles(float *pAngle1, float *pAngle2, float *pAngle3);
    private:
        //vector (to)_(reference)
        vector hip_cog;
        vector foot_hip;

        //gait factors (0..1)
        float offset;
        float & duty; //shared with parent

        //leg dimensions (cm)
        float lPelvis;
        float lUpperLeg;
        float lLowerLeg;

        //angle parameters
        bool isFront;
        bool isLeft;
    };

    //gait parameters
    float xSpeed;
    float ySpeed;
    float rotation;
    float zOffset;
    GaitType gaitType;
    float shakeFactor;
    float duty;
    static const float liftHeight;
    static const float gaitOffsets[GaitType::GAIT_TYPES][4];
    static const float gaitDuty[GaitType::GAIT_TYPES];
    static const float gaitShake[GaitType::GAIT_TYPES];
    static const unsigned long gaitPeriod[GaitType::GAIT_TYPES];
    //timing parameters
    unsigned long tMicros;
    unsigned long periodMicros;
    float phase;
    float gaitTransition;

    //physics parameters
    vector cog_offset;
    Leg *legs[4];
};

#endif