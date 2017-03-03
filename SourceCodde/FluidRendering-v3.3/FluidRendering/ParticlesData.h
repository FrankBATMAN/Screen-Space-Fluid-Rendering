#pragma once
#include <vector>
#include <cstdlib>
#include "datastruct.h"


struct SFluidParticle
{
	std::vector<SVertex> m_ParticleSet;

	SFluidParticle(){}

	void init(int vParticlesNumber, int vRandomSeed)
	{
		srand(vRandomSeed);
		for (int i=0; i<vParticlesNumber; i++)
		{
// 			float t1 = (float)rand();
// 			int   t2 = (float)rand();
// 			float x  = (float)((sinf(t1) * 5.0 + 5.0 + sinf(t2) * 5.0 + 5.0) / 2.0);
// 			float y  = (float)(x * x * x / 200.0 + sin(t2) * 2.5 + 2.5); 
// 			float z  = (float)-((sinf(y) * x / 2.0 + 5.0 + sinf(t1) * 5.0 +5.0) / 2.0);
			float x =3+ (float)(rand()% 60 + 10) / 10;
			float y =3+ (float)(rand()% 40 + 10) / 10;
			float z = (float)-(rand()% 80 + 10) / 10;
			SVertex temp(x, y, z);
			m_ParticleSet.push_back(temp);
		}
	}
	
	void pushParticle(SVertex& vVertex)
	{
		m_ParticleSet.push_back(vVertex);
	}

	void clear()
	{
		m_ParticleSet.clear();
	}
};