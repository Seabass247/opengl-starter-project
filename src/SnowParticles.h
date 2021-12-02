#ifndef __SnowParticles_H__
#define __SnowParticles_H__
#include <random>
#include <functional>

#include "ParticleInterface.h"
class SnowParticles :
    public ParticleInterface
{
private:
    float* positions;
    float* velocities;
    float* accelerations;
    float* orientations;
    int nbrOfParticles;
    int maxParticles;
    std::default_random_engine generator;
    std::normal_distribution<double> velocityDistribution;
    std::uniform_real_distribution<double> angleDistribution;
    std::uniform_real_distribution<double> posXDistribution;
    std::uniform_real_distribution<double> posYDistribution;
    std::uniform_real_distribution<double> posZDistribution;

public:
    void init(int maxParticles = 1000);
    void generate(int maxNewParticles = 5);
    void update(float timeStep = 1.0f / 30.0f);
    void compact();
    float* getPositions() { return positions; }
    float* getVelocities() { return velocities; }
    float* getAccelerations() { return accelerations; }
    float* getOrientations() { return orientations; }
    int getNumberOfParticles(){ return nbrOfParticles;}
    int getMaxParticles() { return maxParticles; }
    SnowParticles();
};

#endif