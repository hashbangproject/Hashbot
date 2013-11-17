// Copyright (C) 2013 Project Hashbang
// Created by Andrew Yang

#ifndef GAIT_H
#define GAIT_H

struct vector
{
    vector();
    vector(const vector &v);
    vector(float x, float y, float z);

    vector operator+(vector other);
    vector operator-(); //unary negate
    vector operator*(float x);
    vector & operator*=(float x);
    vector & operator=(const vector &other); //copy

    void rotate(float theta, vector center);

    float & x;
    float & y;
    float & z;
    float p[3];
};

class StagSystem
{
public:
    StagSystem();
    void setSpeed(float x, float y, float rot, float zOff);
    void moveLegs();
    void getLeg(int leg, float *pAngle1, float *pAngle2, float *pAngle3);
private:
    class Leg
    {
    public:
        Leg(){};
        Leg(float x, float y, float z, float offset, float duty, float lPelvis, float lUpperLeg, float lLowerLeg, bool isFront, bool isLeft);
        void moveLeg(vector position, float theta, float phase, float dPhase);
        //angles returned in radians
        void getAngles(float *pAngle1, float *pAngle2, float *pAngle3);
    private:
        //vector (to)_(reference)
        vector hip_cog;
        vector foot_hip;

        //gait factors (0..1)
        float offset;
        float duty;

        //leg dimensions (cm)
        float lPelvis;
        float lUpperLeg;
        float lLowerLeg;

        //angle parameters
        bool isFront;
        bool isLeft;
    };

    float xSpeed;
    float ySpeed;
    float rotation;
    float zOffset;

    unsigned long tMicros;
    unsigned long periodMicros;
    Leg legs[4];

    static const float liftHeight;
};

#endif
