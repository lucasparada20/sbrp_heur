#ifndef REGRET_INSERTION
#define REGRET_INSERTION

#include "Solution.h"
#include "Move.h"
#include "InsRmvMethodSBRP.h"
#include "OperatorBase.h"

struct RegretMoveSorter
{
	bool operator() (Move * m1, Move * m2)
	{
		return ( m1->DeltaCost < m2->DeltaCost);
	}
};


class RegretInsertionSBRP 
{
	public:
		RegretInsertionSBRP(Prob & pr, InsRmvMethodSBRP & insrmv):
												move_vect(0),_insrmv(insrmv), _moves(0),_used_k(2),_k_regret(2)
		{
			_moves.resize( pr.GetCustomerCount() );
			for(int i = 0 ; i < pr.GetCustomerCount() ; i++)
				_moves[i].resize( pr.GetDriverCount() );
		}

		void SetK(int k){_k_regret = k;}

		void Insert(Sol & s);
		double GetRegretCost(Sol & s, Node * n, Move & m);

	private:
		std::vector< Move* > move_vect;
		InsRmvMethodSBRP & _insrmv;
		std::vector< std::vector< Move > > _moves; //[nodes][driver]

		int _used_k;
		int _k_regret;
		RegretMoveSorter move_sorter;
};

//The class instantiated in main (needs to inherit)
class RegretInsertionOperatorSBRP : public InsertOperator
{
	public:	
		RegretInsertionOperatorSBRP(RegretInsertionSBRP * regret_opt, int k)
				: _regret_opt(regret_opt), _k(k){}
	
		void Insert(Sol & s, bool show) override
		{
			_regret_opt->SetK(_k);
			_regret_opt->Insert(s);
		}
	
	private:
		RegretInsertionSBRP * _regret_opt;
		int _k;
	
};


#endif