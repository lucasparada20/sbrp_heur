#include "RecourseLowerBound.h"
#include "RouteFeasibility.h"
#include "Parameters.h"
#include <algorithm>    // std::reverse
#include <numeric> // std::accumulate

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>

double RecourseLowerBound::Calculate(Prob* prob)
{	
	std::vector<Node*> stations;
	stations.push_back(prob->GetNode(prob->GetNodeCount() - 2));
	for (int i = 0; i < prob->GetCustomerCount(); i++)
		stations.push_back(prob->GetCustomer(i));
	stations.push_back(prob->GetNode(prob->GetNodeCount() - 1));

	double lb = Calculate(prob, stations);

	//printf("L1:%.3lf Cost:%.2lf time:%.3lf\n",l1,lb, time_taken_l1);

	return lb;
}

double RecourseLowerBound::Calculate(Prob* prob, std::vector<Node*>& stations)
{
	feasible = true;
	time_taken_l1 = time_taken_l2 = 0;
	l1 = l2 = 0;
	inf_scenario = -1;

	l1 = 0;

	for (int e = 0;e < prob->GetScenarioCount(); e++)
	{
		double l = Calculate(prob, stations, e);
    
		//printf("scenario:%d cost:%.1lf\n", e,l);
		if(l >= 9999999999)
		{
			l1 = 9999999999;
			feasible = false;
			inf_scenario = e;
			break;
		}
		l1 += l;
   
	}
	l1 /= prob->GetScenarioCount();
	double cost = l1 * Parameters::GetCminEpsilon();

	return cost;

}

double RecourseLowerBound::Calculate(Prob* prob, std::vector<Node*>& stations, int scenario)
{
	int nb_iter = 1;
	int Q = prob->GetDriver(0)->capacity;
	int dmds = 0;
	int lb_left = 0, ub_left = 0, lb_right = 0, ub_right = 0;
	for (size_t l = 0; l < stations.size(); l++)
	{
		if(stations[l]->type != NODE_TYPE_CUSTOMER) continue;

		Node * n = stations[l];
		int dmd = n->demands[scenario];
		dmds += dmd;

		if(dmd > 0) { lb_left += dmd; ub_left += dmd; }
		ub_left += n->w_minus;

		if(dmd < 0) { lb_right -= dmd; ub_right -= dmd; }
		ub_right += n->w_plus;
	}

	while(ub_left+Q < lb_right || lb_left > ub_right+Q)
	{
		Q += prob->GetDriver(0)->capacity;
		nb_iter++;
	}

	//printf("ub_left:%d < lb_right:%d || lb_left:%d > ub_right:%d\n", ub_left, lb_right,lb_left , ub_right);
	if(ub_left+Q  < lb_right || lb_left > ub_right+Q)
		return 9999999999;
	else
		return std::max(0, std::abs(dmds) - Q);
}

double RecourseLowerBound::CalculateWithMinDriverCount(Prob* prob)
{
	std::vector<Node*> stations;
	for (int i = 0; i < prob->GetCustomerCount(); i++)
		stations.push_back(prob->GetCustomer(i));
	return CalculateWithMinDriverCount(prob, stations);
}
double RecourseLowerBound::CalculateWithMinDriverCount(Prob* prob, std::vector<Node*>& stations)
{
	//first sum the demands
	std::vector<int> dmds(prob->GetScenarioCount(), 0);
	std::vector<int> lb_left(prob->GetScenarioCount(), 0);
	std::vector<int> ub_left(prob->GetScenarioCount(), 0);
	std::vector<int> lb_right(prob->GetScenarioCount(), 0);
	std::vector<int> ub_right(prob->GetScenarioCount(), 0);
	for (int e = 0;e < prob->GetScenarioCount(); e++)
		for (size_t l = 0; l < stations.size(); l++)
		{
			if(stations[l]->type != NODE_TYPE_CUSTOMER) continue;

			Node * n = stations[l];
			int dmd = n->demands[e];
			dmds[e] += dmd;

			if(dmd > 0) { lb_left[e] += dmd; ub_left[e] += dmd; }
			ub_left[e] += n->w_minus;

			if(dmd < 0) { lb_right[e] -= dmd; ub_right[e] -= dmd; }
			ub_right[e] += n->w_plus;
		}

	//get the minimum number of vehicles
	int Q = prob->GetDriver(0)->capacity;
	int nb_drivers = 1;
	for (int e = 0;e < prob->GetScenarioCount(); e++)
		while(ub_left[e]+Q*nb_drivers < lb_right[e] || lb_left[e] > ub_right[e]+Q*nb_drivers)
			nb_drivers++;

	double lb = 0;
	for (int e = 0;e < prob->GetScenarioCount(); e++)
		lb += std::max(0, std::abs(dmds[e]) - Q*nb_drivers);
	lb /= prob->GetScenarioCount();
	lb *= Parameters::GetCminEpsilon();

	return lb;
}


int RecourseLowerBound::GetDriverCount(Prob* prob)
{	
	std::vector<Node*> stations;
	for (int i = 0; i < prob->GetCustomerCount(); i++)
		stations.push_back(prob->GetCustomer(i));
	return GetDriverCount(prob, stations);
}

int RecourseLowerBound::GetDriverCount(Prob* prob, std::vector<Node*>& stations)
{
	int nb_drivers=1;
	for (int e=0;e<prob->GetScenarioCount();e++)
	{
		int Q = prob->GetDriver(0)->capacity * nb_drivers;
		int dmds = 0;
		int lb_left = 0, ub_left = 0, lb_right = 0, ub_right = 0;
		for (size_t l = 0; l < stations.size(); l++)
		{
			if(stations[l]->type != NODE_TYPE_CUSTOMER) continue;
			Node * n = stations[l];
			
			//n->Show();
			
			int dmd = n->demands[e];
			dmds += dmd;

			if(dmd > 0) { lb_left += dmd; ub_left += dmd; }
			ub_left += n->w_minus;

			if(dmd < 0) { lb_right -= dmd; ub_right -= dmd; }
			ub_right += n->w_plus;
		}

		while(ub_left+Q < lb_right || lb_left > ub_right+Q)
		{
			Q += prob->GetDriver(0)->capacity;
			nb_drivers++;
		}
	}
	return nb_drivers;
}

void RecourseLowerBound::SetWorstScenario(Prob* prob){
	std::vector<Node*> stations;
	stations.push_back(prob->GetNode(prob->GetNodeCount() - 2));
	for (int i = 0; i < prob->GetCustomerCount(); i++)
		stations.push_back(prob->GetCustomer(i));
	stations.push_back(prob->GetNode(prob->GetNodeCount() - 1));

	double worst_cost = -1;
	int worst_scenario = -1;
	for (int e=0;e<prob->GetScenarioCount();e++)
	{
		double cost = Calculate(prob, stations, e);
		if(cost > worst_cost)
		{
			worst_cost = cost;
			worst_scenario = e;
		}
	}
	Parameters::SetWorstScenario(worst_scenario);
	printf("worst_scenario:%d cost:%.1lf\n",worst_scenario,worst_cost);

}

void RecourseLowerBound::SortFromWorstScenarios(Prob* prob)
{
	std::vector<Node*> stations;
	stations.push_back(prob->GetNode(prob->GetNodeCount() - 2));
	for (int i = 0; i < prob->GetCustomerCount(); i++)
		stations.push_back(prob->GetCustomer(i));
	stations.push_back(prob->GetNode(prob->GetNodeCount() - 1));

	RecourseLowerBound rec;

	std::vector<LBandIndex> Container;
	std::vector<double> ScenarioL(prob->GetScenarioCount(), -1);
	for (size_t e = 0; e < ScenarioL.size(); e++)
	{
		LBandIndex element;
		element.Index = e;
		element.LB = ScenarioL[e] = rec.Calculate(prob, stations, e);
		Container.push_back(element);
	}
	
	std::vector<double> aux(prob->GetScenarioCount(), -1);
	for (size_t e = 0; e < Container.size(); e++)
		aux[e] = Container[e].LB;

	std::sort(aux.begin(), aux.end());
	std::reverse(aux.begin(), aux.end());
	
	for (size_t e = 0; e < aux.size(); e++)
	{
		for (size_t i = 0; i < Container.size(); i++)
		{
			if (aux[e] == Container[i].LB)
				if (Container[i].AlreadyAdded == false)
				{
					prob->AddSortedScenarios(Container[i].Index);
					Container[i].AlreadyAdded = true;
					break;
				}
		}

	}
	
}
