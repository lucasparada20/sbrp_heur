#include "RegretInsertionSBRP.h"
#include <algorithm>

void RegretInsertionSBRP::Insert(Sol & s)
{
	_used_k = std::min(s.GetDriverCount(), _k_regret);
	if((int)_moves[0].size() < s.GetDriverCount()) //Drivers were added
	{
		for(int i = 0 ; i < s.GetCustomerCount() ; i++)
			_moves[i].resize( s.GetDriverCount() );
	}

	//printf("\n\nRegretInsertion k:%d\n",_k_regret);
	std::vector<Node*> toRemove(0); toRemove.reserve( s.GetCustomerCount() );
	std::vector<Node*> refused(0); refused.reserve( s.GetCustomerCount() );
	s.Update();
	for(int i=0;i<s.GetUnassignedCount();i++)
		for(int j = 0 ; j < s.GetDriverCount() ; j++)
		{
			Node * n = s.GetUnassigned(i);
			Driver * d = s.GetDriver(j);
			_insrmv.InsertCost(s, n, d, _moves[n->id][d->id]);
		}

	while(s.GetUnassignedCount() > 0)
	{
		//printf("\nIteration:%d\n",s.GetUnassignedCount());
		Move best;
		best.IsFeasible = false;
		double maxRegret = -INFINITE;

		for(int i=0;i<s.GetUnassignedCount();i++)
		{
			Move m;
			double regret = GetRegretCost(s, s.GetUnassigned(i), m);
			//printf("regret:%.lf maxRegret:%.lf feas:%d dmd:%d\tn:%d\n",
			//		regret, maxRegret, (int)m.IsFeasible,m.n->demand, m.n->id);

			if(m.IsFeasible && (regret > maxRegret || (regret == maxRegret && m.DeltaCost < best.DeltaCost)))
			{
				maxRegret = regret;
				best = m;
			}
			else if (!m.IsFeasible)
				toRemove.push_back( s.GetUnassigned(i) );
		}


		if(best.IsFeasible)
		{
			best.from = NULL;
			_insrmv.ApplyInsertMove(s,best);
			s.Update(best.to);
		}
		//else
		//	printf("No feasible move found\n");

		for(size_t i = 0 ; i < toRemove.size() ; i++)
		{
			refused.push_back(toRemove[i]);
			s.RemoveFromUnassigneds(toRemove[i]);
		}
		toRemove.clear();

		for(int i=0;i<s.GetUnassignedCount();i++)
		{
			Node * n = s.GetUnassigned(i);
			if(_moves[n->id][best.to->id].IsFeasible)
				_insrmv.InsertCost(s, n, best.to, _moves[n->id][best.to->id]);
		}

	}//end while

	for(size_t i = 0 ; i < refused.size() ; i++)
		s.AddToUnassigneds( refused[i] );
}

double RegretInsertionSBRP::GetRegretCost(Sol & s, Node * n, Move & m)
{
	int nbfeasible = 0;
	move_vect.clear(); move_vect.reserve( s.GetDriverCount() );
	for(int j = 0 ; j < s.GetDriverCount() ; j++)
	{
		move_vect.push_back(&_moves[n->id][ s.GetDriver(j)->id ]);
		if(_moves[n->id][ s.GetDriver(j)->id ].IsFeasible)
			nbfeasible++;
	}
	//printf("Node:%d nbfeasible:%d ", n->id,nbfeasible);
	partial_sort(move_vect.begin(), move_vect.begin()+_used_k,  move_vect.end(), move_sorter);

	double cost = 0;
	m = *move_vect[0];
	for(int j = 1 ; j < _used_k ; j++)
		cost += move_vect[j]->DeltaCost - move_vect[0]->DeltaCost;
	return cost;
}