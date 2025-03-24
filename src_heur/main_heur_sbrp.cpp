#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <csignal>
#include <string.h> //strcm

#include "NodeSBRP.h"
#include "DriverSBRP.h"
#include "LoadSBRP.h"

#include "Parameters.h"
#include "ProblemDefinition.h"
#include "RecourseLowerBound.h"
#include "RouteFeasibility.h"
#include "Solution.h"
#include "CostFunctionSBRP.h"
#include "SequentialInsertionSBRP.h"
#include "RegretInsertionSBRP.h"
#include "RelatednessRemoveSBRP.h"
#include "AlnsSBRP.h"

#include <ctime>
#include <fstream>
#include <iostream>

int main(int arg, char ** argv)
{
	
	if(arg == 1)
	{
		printf("usage: executable, instance_file, epsilon, delta, cuts_type, instance_type, algorithm optional:initial_solution_file \n");
		printf("Instance_type: dins or pcg\n");
		printf("Cuts: P&L=1 Benders=2 Hybrid=3\n");
		printf("Algorithm: DL-shaped=dl Multicut=m(Only accepts Benders Opt Cuts=6)\n");
		printf("exiting.\n");
		exit(1);
	}

	Parameters param;
	param.Read(arg,argv);
	
	if(Parameters::GetIterations() < 1)
	{
		printf("Need more than 1 iterarion. Exiting ...\n"); exit(1);
	}
	

	Prob pr;
	LoadSBRP Load;

	if(std::strcmp(Parameters::GetInstanceType(),"dins")==0) 
		Load.Load_dins(pr, Parameters::GetInstanceFileName());
	else if(std::strcmp(Parameters::GetInstanceType(),"pcg")==0) 
		Load.Load_pcg(pr, Parameters::GetInstanceFileName());
	else {
		printf("Wrong file type. Exiting ... \n"); exit(1);
	}

	RecourseLowerBound::SetWorstScenario(&pr);
	RecourseLowerBound::SortFromWorstScenarios(&pr);
 
	//Instantiation of ALNS operators
	CostFunctionSBRP cost_func;
	
	Sol sol(&pr,&cost_func);
	sol.PutAllNodesToUnassigned();
	
	InsRmvMethodSBRP method(pr);
	SequentialInsertionSBRP seq(method);
	RegretInsertionSBRP regret(pr,method);
	RegretInsertionOperatorSBRP regret2(&regret,3);
	RegretInsertionOperatorSBRP regretk(&regret,pr.GetDriverCount());

	RemoveRandomSBRP random_remove;

	RelatednessRemoveSBRP related_remove(pr.GetDistances());
	
	seq.Insert(sol); //Sequential insertion of nodes to build an initial solution and store in sol 

	ALNS alns;
	alns.AddInsertOperator(&seq);
	//----------------------------------
	alns.AddInsertOperator(&regret2);
	alns.AddInsertOperator(&regretk);
	//---------------------------------
	alns.AddRemoveOperator(&random_remove);
	alns.AddRemoveOperator(&related_remove);

	seq.Insert(sol);
	
	//Optimize
	alns.SetTemperatureIterInit(0);
	alns.SetTemperature(0.99);
	//alns.SetIterationCount(750000);//Remember to set a lot of iterations
	alns.SetIterationCount( Parameters::GetIterations() );
	
	clock_t start_time = clock();
	
	
	alns.Optimize(sol);
	//BestSolutionList best_sol_list(&pr,100);
	//alns.Optimize(sol,&best_sol_list);
	//for(int k=0;k<best_sol_list.GetSolutionCount();k++)
	//{
		//printf("Alns solution:%d\n",k);
		//Sol& s1 = *best_sol_list.GetSolution(k);
		//s1.Show();
	//}	
	
	clock_t end_time = clock();
	
	double elapsed_seconds = (double) (end_time - start_time) / CLOCKS_PER_SEC;
	
	sol.Update();
	sol.Show();
	
	double heur_ub_distance = sol.GetTotalDistances();
	double heur_ub_recourse = sol.GetTotalRecourse();
	double heur_ub = heur_ub_distance + heur_ub_recourse;
	int heur_nb_drivers = sol.GetUsedDriverCount();
	printf("time:%.2lf\n", elapsed_seconds);
	printf("UB Heur:%.3lf dist:%.3lf rec:%.3lf drv:%d\n", heur_ub, heur_ub_distance, heur_ub_recourse,heur_nb_drivers); 	

	return 0;
}


