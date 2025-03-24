
#ifndef RECOURSE_LOWER_BOUND
#define RECOURSE_LOWER_BOUND

#include <vector>
#include "NodeSBRP.h"
#include "DriverSBRP.h"
#include "ProblemDefinition.h"

class RecourseLowerBound
{
public:
	double Calculate(Prob* prob);
	double Calculate(Prob* prob, std::vector<Node*>& stations);
	static double Calculate(Prob* prob, std::vector<Node*>& stations, int scenario);

	double CalculateWithMinDriverCount(Prob* prob);
	double CalculateWithMinDriverCount(Prob* prob, std::vector<Node*> & stations);

	static int GetDriverCount(Prob* prob);
	static int GetDriverCount(Prob* prob, std::vector<Node*>& stations);

	static void SetWorstScenario(Prob* prob);
	static void SortFromWorstScenarios(Prob* prob);

	double l1, l2;
	double time_taken_l1;
	double time_taken_l2;
	bool feasible;
	int inf_scenario;
	int nb_drivers;

	struct LBandIndex
	{
		double LB;
		int Index;
		bool AlreadyAdded = false;
	};
  
};



#endif
