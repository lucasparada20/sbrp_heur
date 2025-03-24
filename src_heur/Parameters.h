#ifndef _PARAMETERS_H
#define _PARAMETERS_H

#include <time.h>
#include <math.h>

#define OPT_CUT_TYPE_PL 1
#define OPT_CUT_TYPE_BENDERS 2
#define OPT_CUT_TYPE_HYBRID 3

class Parameters
{

public:
	void Read(int arg, char ** argv);

	static double GetEpsilon() { return Epsilon; }
	static double GetDelta() { return delta; }
	static double GetCmin() { return Cmin; }
	static double GetCminEpsilon(){return ceil(Cmin*Epsilon);}
	static int GetWorstScenario() { return WorstScenario; }
	static int GetOppositeScenario() { return OppositeScenario; }
	static char* GetInstanceFileName(){return instance_file;}
	static char* GetInstanceType(){return instance_type;}
	static int GetIterations(){ return Iterations; }

	static void SetEpsilon(double e) { Epsilon = e; }
	static void SetDelta(double g) { delta = g; }
	static void SetCmin(double c) { Cmin = c; }
	static void SetWorstScenario(int e) { WorstScenario = e; }
	static void SetOppositeScenario(int e) { OppositeScenario = e; }

private:

	static double delta;
	static double Epsilon;
	static double Cmin;
	static int WorstScenario;
	static int OppositeScenario;

	static char * instance_file;
	static char * instance_type;
	
	static int Iterations;
};


#endif
