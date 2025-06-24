#ifndef LOAD_SBRP
#define LOAD_SBRP

#include "ProblemDefinition.h"
#include "NodeSBRP.h"
#include "DriverSBRP.h"

class LoadSBRP
{
	public:
		void Load_dins(Prob & pr, const char * filename, int driver_count);
		void Load_pcg(Prob & pr, const char * filename, int driver_count);
};

#endif