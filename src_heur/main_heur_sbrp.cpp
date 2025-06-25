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
		printf("usage: executable, instance_file, epsilon, delta, instance_type\n");
		printf("Instance_type: dins or pcg\n");
		printf("exiting.\n");
		exit(1);
	}

	Parameters param;
	param.Read(arg,argv);	
	
	Prob pr_first;
	Prob pr_temp;
	LoadSBRP Load;

	if(std::strcmp(Parameters::GetInstanceType(),"dins")==0) 
		Load.Load_dins(pr_first, Parameters::GetInstanceFileName(),1);
	else if(std::strcmp(Parameters::GetInstanceType(),"pcg")==0) 
		Load.Load_pcg(pr_first, Parameters::GetInstanceFileName(),1);
	else {
		printf("Wrong file type. Exiting ... \n"); exit(1);
	}

	RecourseLowerBound::SetWorstScenario(&pr_first);
	RecourseLowerBound::SortFromWorstScenarios(&pr_first);
 
	//Instantiation of ALNS operators
	CostFunctionSBRP cost_func;
	
	Sol sol(&pr_first,&cost_func);
	sol.PutAllNodesToUnassigned();
	
	InsRmvMethodSBRP method1(pr_first);
	SequentialInsertionSBRP seq1(method1);
	RemoveRandomSBRP random_remove1;
	RelatednessRemoveSBRP related_remove1(pr_first.GetDistances());
	
	// An Alns run to find a feasible solution with some drivers
	ALNS alns1;
	//=============================//
	alns1.AddInsertOperator(&seq1);
	//=============================//
	//alns1.AddInsertOperator(&regret2);
	//alns1.AddInsertOperator(&regretk);
	//====================================//
	alns1.AddRemoveOperator(&random_remove1);
	alns1.AddRemoveOperator(&related_remove1);

	seq1.Insert(sol,false); //Sequential insertion of nodes to build an initial solution and store in sol 
	
	//Optimize
	alns1.SetTemperatureIterInit(0);
	alns1.SetTemperature(0.9995);
	alns1.SetAcceptationGap(1.1);
	alns1.SetIterationCount( 1000 );
	alns1.Optimize(sol);
	sol.Update();
	sol.Show();	
	double cost = sol.GetCost();
	int best_driver_count = 1;
	
	for(int i=2;i<=10;i++)
	{
		pr_temp.Clear();
		if(std::strcmp(Parameters::GetInstanceType(),"dins")==0) 
			Load.Load_dins(pr_temp, Parameters::GetInstanceFileName(),i);
		else if(std::strcmp(Parameters::GetInstanceType(),"pcg")==0) 
			Load.Load_pcg(pr_temp, Parameters::GetInstanceFileName(),i);
		else {
			printf("Wrong file type. Exiting ... \n"); exit(1);
		}
		RecourseLowerBound::SetWorstScenario(&pr_temp);
		RecourseLowerBound::SortFromWorstScenarios(&pr_temp);
		
		Sol sol_i(&pr_temp,&cost_func);
		sol_i.PutAllNodesToUnassigned();

		InsRmvMethodSBRP method_i(pr_temp);
		SequentialInsertionSBRP seq_i(method_i);
		RegretInsertionSBRP regret_i(pr_temp,method_i);
		RemoveRandomSBRP random_remove_i;
		RelatednessRemoveSBRP related_remove_i(pr_temp.GetDistances());

		// An Alns run to find a feasible solution with some drivers
		ALNS alns_i;
		//=============================//
		alns_i.AddInsertOperator(&seq_i);
		//=============================//
		//alns1.AddInsertOperator(&regret2);
		//alns1.AddInsertOperator(&regretk);
		//====================================//
		alns_i.AddRemoveOperator(&random_remove_i);
		alns_i.AddRemoveOperator(&related_remove_i);

		seq_i.Insert(sol_i,false); //Sequential insertion of nodes to build an initial solution and store in sol 

		//Optimize
		alns_i.SetTemperatureIterInit(0);
		alns_i.SetTemperature(0.9995);
		alns_i.SetAcceptationGap(1.1);
		alns_i.SetIterationCount( 1000 );
		alns_i.Optimize(sol_i);
		sol_i.Update();
		
		double cost_i = sol_i.GetCost();
		
		printf("Curr_cost:%.1lf new_cost:%.1lf\n",cost,cost_i);
		if (sol_i.IsFeasible() && cost_i < cost)
		{
			printf("Found better solution with loaded nb drv:%d!\n",pr_temp.GetDriverCount());
			sol_i.Show();

			cost = cost_i;
			best_driver_count = sol_i.GetDriverCount();
			sol = sol_i;
		}		
	}
	
	Prob pr;
	printf("Best solution found:%.1lf with nb drvs:%d ... Reloading for main Alns ... \n",
		cost, best_driver_count);
	if(std::strcmp(Parameters::GetInstanceType(),"dins")==0) 
		Load.Load_dins(pr, Parameters::GetInstanceFileName(),best_driver_count);
	else if(std::strcmp(Parameters::GetInstanceType(),"pcg")==0) 
		Load.Load_pcg(pr, Parameters::GetInstanceFileName(),best_driver_count);
	
	sol.SetProb(&pr); // Before updating, you need to set the right prob object
	RecourseLowerBound::SetWorstScenario(&pr);
	RecourseLowerBound::SortFromWorstScenarios(&pr);
		
	InsRmvMethodSBRP method(pr);
	SequentialInsertionSBRP seq(method);
	RemoveRandomSBRP random_remove;
	RelatednessRemoveSBRP related_remove(pr.GetDistances());

	ALNS alns;
	alns.AddInsertOperator(&seq);
	//=============================//
	alns.AddRemoveOperator(&random_remove);
	alns.AddRemoveOperator(&related_remove);
	//=====================================//
	
	// The main Alns run
	alns.SetTemperatureIterInit(0);
	alns.SetTemperature(0.9995);
	alns.SetAcceptationGap(1.1);	
	alns.SetIterationCount( Parameters::GetIterations() );
	clock_t start_time = clock();
	for(int i=0;i<10;i++)
	{
		printf("Alns run:%d\n",i+1);
		Sol s = sol;
		seq.Insert(s, false);
		alns.Optimize(sol);

		if(sol.GetCost() > s.GetCost())
			sol = s;
	}
	clock_t end_time = clock();
	
	printf("Solution refining by calculating with all scenarios ...\n");
	alns.SetIterationCount(5000);
	method.calculate_with_all_scenarios = true;
	alns.Optimize(sol);
	
	//BestSolutionList best_sol_list(&pr,100);
	//alns.Optimize(sol,&best_sol_list);
	//for(int k=0;k<best_sol_list.GetSolutionCount();k++)
	//{
		//printf("Alns solution:%d\n",k);
		//Sol& s1 = *best_sol_list.GetSolution(k);
		//s1.Show();
	//}	
	
	
	
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


