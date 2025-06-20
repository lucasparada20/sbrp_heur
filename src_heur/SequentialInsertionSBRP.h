#ifndef SEQUENTIAL_INSERTION
#define SEQUENTIAL_INSERTION

#include "Solution.h"
#include "Move.h"
#include "InsRmvMethodSBRP.h"
#include "OperatorBase.h"
#include "RouteFeasibility.h"
#include "RecourseLowerBound.h"

#include <algorithm> 

class SequentialInsertionSBRP : public InsertOperator
{
	public:
		SequentialInsertionSBRP(InsRmvMethodSBRP & insrmv): _insrmv(insrmv){}

	void Insert(Sol & s, bool show) override;
	void InsertOld(Sol & s);
	void FillMoveVec(Sol & s, Node * n, Driver * d, std::vector<Move> & moveVec);

	private:
		InsRmvMethodSBRP & _insrmv;
		std::vector<Node*> path;

};


#endif