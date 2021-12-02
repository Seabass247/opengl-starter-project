#ifndef __PARTICLEINTERFACE_H__
#define __PARTICLEINTERFACE_H__

class ParticleInterface {
	virtual void init(int maxParticles) = 0;
	virtual void generate(int maxNewParticles) = 0;
	virtual void update(float timeStep) = 0;
	virtual void compact() = 0;
	virtual float* getPositions() = 0;
	virtual float* getVelocities() = 0;
	virtual float* getAccelerations() = 0;
	virtual float* getOrientations() = 0;
	virtual int getNumberOfParticles() = 0;
	virtual int getMaxParticles() = 0;
};

#endif
