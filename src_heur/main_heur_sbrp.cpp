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
	
	Prob pr_temp;
	Prob pr;
	Prob pr_while;
	LoadSBRP Load;

	if(std::strcmp(Parameters::GetInstanceType(),"dins")==0) 
		Load.Load_dins(pr_temp, Parameters::GetInstanceFileName(),1);
	else if(std::strcmp(Parameters::GetInstanceType(),"pcg")==0) 
		Load.Load_pcg(pr_temp, Parameters::GetInstanceFileName(),1);
	else {
		printf("Wrong file type. Exiting ... \n"); exit(1);
	}

	RecourseLowerBound::SetWorstScenario(&pr_temp);
	RecourseLowerBound::SortFromWorstScenarios(&pr_temp);
 
	//Instantiation of ALNS operators
	CostFunctionSBRP cost_func;
	
	Sol sol(&pr_temp,&cost_func);
	sol.PutAllNodesToUnassigned();
	
	InsRmvMethodSBRP method1(pr_temp);
	SequentialInsertionSBRP seq1(method1);
	RegretInsertionSBRP regret(pr_temp,method1);
	RegretInsertionOperatorSBRP regret2(&regret,3);
	RegretInsertionOperatorSBRP regretk(&regret,pr_temp.GetDriverCount());
	RemoveRandomSBRP random_remove1;
	RelatednessRemoveSBRP related_remove1(pr_temp.GetDistances());
	
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
	
	int drv = 1;
	while(!sol.IsFeasible())
	{
		drv++;
		pr_while.Clear();
		if(std::strcmp(Parameters::GetInstanceType(),"dins")==0) 
			Load.Load_dins(pr_while, Parameters::GetInstanceFileName(),drv);
		else if(std::strcmp(Parameters::GetInstanceType(),"pcg")==0) 
			Load.Load_pcg(pr_while, Parameters::GetInstanceFileName(),drv);
		
		RecourseLowerBound::SetWorstScenario(&pr_while);
		RecourseLowerBound::SortFromWorstScenarios(&pr_while);
		
		printf("No feasible solution. Restarting Alns with nb drvs:%d\n",pr_while.GetDriverCount());

		InsRmvMethodSBRP method2(pr_while);
		SequentialInsertionSBRP seq2(method2);
		RegretInsertionSBRP regret(pr_while,method2);
		RemoveRandomSBRP random_remove2;
		RelatednessRemoveSBRP related_remove2(pr_while.GetDistances());
		
		// An Alns run to find a feasible solution with some drivers
		ALNS alns2;
		//=============================//
		alns2.AddInsertOperator(&seq2);
		//=============================//
		alns2.AddRemoveOperator(&random_remove2);
		alns2.AddRemoveOperator(&related_remove2);
		//=============================//
		
		Sol s(&pr_while,&cost_func);
		s.PutAllNodesToUnassigned();
		seq2.Insert(s,false); //Sequential insertion of nodes to build an initial solution and store in sol 
		
		//Optimize
		alns2.SetTemperatureIterInit(0);
		alns2.SetTemperature(0.9995);
		alns2.SetAcceptationGap(1.1);
		alns2.SetIterationCount( 1000 );
		alns2.Optimize(s);
		s.Update();
		s.Show();
		if(s.IsFeasible())
			sol = s; //They have different prob objects ...
	}
	
	
	printf("Feasible solution found with nb drvs:%d Testing with more drivers ... \n",sol.GetUsedDriverCount());
	if(std::strcmp(Parameters::GetInstanceType(),"dins")==0) 
		Load.Load_dins(pr, Parameters::GetInstanceFileName(),sol.GetUsedDriverCount()+2);
	else if(std::strcmp(Parameters::GetInstanceType(),"pcg")==0) 
		Load.Load_pcg(pr, Parameters::GetInstanceFileName(),sol.GetUsedDriverCount()+2);
	
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
	
	// Alns run with more drivers
	alns.SetIterationCount( 1000 );
	
	Sol new_sol(&pr,&cost_func);
	new_sol.PutAllNodesToUnassigned();
	seq.Insert(new_sol, false);
	alns.Optimize(new_sol);
	new_sol.Update();
	printf("Cost with more drivers:%.1lf drv:%d\n",new_sol.GetCost(), new_sol.GetUsedDriverCount());
	
	if(new_sol.GetCost() < sol.GetCost())
	{
		printf("Starting main Alns with solution with more drivers!\n");
		sol = new_sol;
	} else
		printf("Will do main Alns run with nb drv:%d!\n",sol.GetUsedDriverCount());
		
	
	// The main Alns run
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


