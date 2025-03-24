#ifndef SEQUENTIAL_INSERTION
#define SEQUENTIAL_INSERTION

#include "Solution.h"
#include "Move.h"
#include "InsRmvMethodSBRP.h"
#include "OperatorBase.h"

class SequentialInsertionSBRP : public InsertOperator
{
	public:
		SequentialInsertionSBRP(InsRmvMethodSBRP & insrmv): _insrmv(insrmv){}

	void Insert(Sol & s) override;

	private:
		InsRmvMethodSBRP & _insrmv;

};


#endif