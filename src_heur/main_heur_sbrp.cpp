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
	
	srand(0);
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

	ALNS alns;
	alns.AddInsertOperator(&seq);
	//----------------------------------
	alns.AddInsertOperator(&regret2);
	alns.AddInsertOperator(&regretk);
	//---------------------------------
	alns.AddRemoveOperator(&random_remove);
	alns.AddRemoveOperator(&related_remove);

	seq.Insert(sol,false); //Sequential insertion of nodes to build an initial solution and store in sol 
	
	//Optimize
	alns.SetTemperatureIterInit(0);
	alns.SetTemperature(0.99);
	alns.SetIterationCount( Parameters::GetIterations() );
	
	clock_t start_time = clock();
	
	for(int i=0;i<10;i++)
	{
		Sol s = sol;
		seq.Insert(s, true);
		alns.Optimize(sol);

		if(sol.GetCost() > s.GetCost())
			sol = s;
	}
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
	
	
	// Find the position of the last '/'
    size_t lastSlash =std::string(Parameters::GetInstanceFileName()).find_last_of('/');
    // Find the position of the last '.'
    size_t lastDot = std::string(Parameters::GetInstanceFileName()).find_last_of('.');
    // Extract the substring between the last '/' and the last '.'
    std::string instance_name_str = std::string(Parameters::GetInstanceFileName()).substr(lastSlash + 1, lastDot - lastSlash - 1);
	
	std::string re_file_name_str = "results/re_" +  instance_name_str + ".txt";
	std::ofstream re_file( re_file_name_str );
	
	if(!re_file.is_open())
	{
		printf("Could not open re file:%s\n",re_file_name_str.c_str());
		exit(1);
	}
	
	re_file 
		<< instance_name_str << ";" 
		<< heur_ub << ";"
		<< heur_ub_distance << ";"
		<< heur_ub_recourse << ";"
		<< heur_nb_drivers << ";"
		<< elapsed_seconds << "\n";
		
	re_file.close();

	return 0;
}


