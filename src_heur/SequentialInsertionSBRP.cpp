#include "SequentialInsertionSBRP.h"

void SequentialInsertionSBRP::FillMoveVec(Sol & s, Node * n, Driver * d, std::vector<Move> & moveVec)
{
	int pos = 0;
	Node * prev = s.GetNode(d->StartNodeID);
	while (prev->type != NODE_TYPE_END_DEPOT)
	{
		Node * next = s.Next[prev->id];
		if (prev->type != NODE_TYPE_START_DEPOT)
			iter_swap(path.begin() + pos, path.begin() + pos + 1);

		double deltaDist = s.GetDist(prev, n) + s.GetDist(n, next) - s.GetDist(prev, next);
		
		bool is_feas = RouteFeasibility::IsFeasible(s.GetProb(), path);
		
		if(!is_feas)
		{
			prev = next; pos++; continue;
		}

		bool hasZeroRec = RouteFeasibility::HasZeroHC(s.GetProb(),path);

		//The move is feasible! store it.
		Move mo;
		mo.DeltaDistance = deltaDist;
		mo.DeltaRec = (hasZeroRec && d->curRecourse > 0) ? (-1 * d->curRecourse) : 
		  (hasZeroRec && d->curRecourse == 0) ? 0 : 9999;  
		mo.DeltaCost = mo.DeltaDistance + mo.DeltaRec; // if mo.DeltaRec > 0, then DeltaCost = mo.DeltaDistance + INFINITE 
		mo.IsFeasible = true;
		mo.n = n;
		mo.to = d;
		mo.pos = pos;
		mo.move.prev = prev;
		
		//if(mo.DeltaRec>0) 
			//mo.Show();
		
		moveVec.push_back(mo);
		
		//printf("Feas insertion d:%d n:%d zeroRec:%d Path:\n",d->id,n->no,hasZeroRec);
		//for(int k=0;k<path.size();k++)
		//	printf("%d-",path[k]->no);
		//printf("\n");
		
		prev = next; pos++;
	}	
}

void SequentialInsertionSBRP::Insert(Sol & s, bool show)
{
	s.Update();
	std::vector<Node*> nodes; //list of customers to insert
	std::vector<Node*> refused; //list of infeasible customers
	_insrmv.FillInsertionList(s, nodes);
	//printf("SeqInsertion %d\n", s.GetUnassignedCount());
	
	clock_t startTime = clock();
	for(size_t i=0;i<nodes.size();i++)
	{
		Node * n = nodes[i];
		if (show && i % 10 == 0)
		{
			clock_t endTime = clock();
			double elapsedSeconds = (double)(endTime - startTime) / CLOCKS_PER_SEC;
			printf("SeqInsetion inserting node:%d ElapsedSeconds:%.1lf\n", n->id, elapsedSeconds);
		}
		
		std::vector<Move> moveVec; moveVec.reserve( s.GetNodeCount() );
		bool has_seen_empty_driver = false;
		for(int j = 0 ; j < s.GetDriverCount() ; j++)
		{
			Driver * d = s.GetDriver(j);
			path.clear(); path.reserve( s.GetRouteLength( s.GetDriver(j)) );
			Node * prev = s.GetNode( d->StartNodeID );
			path.push_back(prev); 
			path.push_back(n);
			while(prev->type != NODE_TYPE_END_DEPOT)
			{
				Node * next = s.Next[ prev->id ];
				path.push_back(next);
				prev = next;
			}
			
			if(RecourseLowerBound::GetDriverCount(s.GetProb(), path) > 1) continue;
			
			if(s.GetRouteLength( s.GetDriver(j)) == 0 && has_seen_empty_driver) continue;
			
			FillMoveVec(s, n, d, moveVec);
			
			if(s.GetRouteLength( s.GetDriver(j)) == 0)
				has_seen_empty_driver = true;
		}
		
		// Check the best moves 
		
		//printf("looping at:%zu moves:%zu\n",i,moveVec.size());
		std::sort(moveVec.begin(), moveVec.end());
		int bestRec = 9999;
		Move best;
		best.IsFeasible = true;
		best.DeltaCost = INFINITE;
		
		//Find the best delta Recourse of the moves
		for(int j=0;j<std::min(50,(int)moveVec.size());j++)
		{
			Move mo = moveVec[j];
			Driver * d = mo.to;
			path.clear(); path.reserve( s.GetRouteLength( s.GetDriver(j)) );
			
			//insert the node 'n' in path at position mo.pos
			Node * prev = s.GetNode( d->StartNodeID );
			path.push_back(prev); 
			int cntr = 0;
			if(mo.pos == 0) path.push_back(n);
			while(prev->type != NODE_TYPE_END_DEPOT)
			{
				Node * next = s.Next[ prev->id ];
				path.push_back(next);
				cntr++;
				if(mo.pos == cntr) path.push_back(n);
				prev = next;
			}
			
			int rec = mo.DeltaRec <= 0 ? 0 : RouteFeasibility::RecourseCost(s.GetProb(),path);
			
			mo.DeltaRec = mo.DeltaRec <= 0 ? mo.DeltaRec : rec - d->curRecourse; //DeltaRec can be negative due to a helpful insertion of a station in the middle of a route
			
			if(mo.DeltaDistance + mo.DeltaRec < best.DeltaCost) // Just if its feasible and then store in an std::vector<Move> and sort
			{
				mo.DeltaCost = mo.DeltaRec + mo.DeltaDistance; 
				best = mo;
			}
			
			//Test the next move in the vector to check if is not better. If its indeed not better, then break!
			if( j+1 < (int)moveVec.size() )
			{
				Move nextMo = moveVec[j+1];
				Driver * nextD = nextMo.to;
				path.clear(); path.reserve( s.GetRouteLength( s.GetDriver(j)) );
				
				//insert the node 'n' in path at position mo.pos
				Node * prev = s.GetNode( nextD->StartNodeID );
				path.push_back(prev); 
				int cntr = 0;
				if(nextMo.pos == 0) path.push_back(n);
				while(prev->type != NODE_TYPE_END_DEPOT)
				{
					Node * next = s.Next[ prev->id ];
					path.push_back(next);
					cntr++;
					if(nextMo.pos == cntr) path.push_back(n);
					prev = next;
				}

				int nextRec = nextMo.DeltaRec <= 0 ? 0 : RouteFeasibility::RecourseCost(s.GetProb(), path);	
				
				nextMo.DeltaRec = nextMo.DeltaRec <= 0 ? nextMo.DeltaRec : rec - nextD->curRecourse; //DeltaRec can be negative due to a helpful insertion of a station in the middle of a route
				
				if(nextMo.DeltaDistance + nextMo.DeltaRec > best.DeltaCost)
					break;
			}
				
		}
	
		best.from = NULL;
		//_insrmv.ApplyInsertMove(s, best);
		//s.Update(best.to);
		
		if(best.IsFeasible)
		{
			_insrmv.ApplyInsertMove(s, best);
			s.Update(best.to);
		}
		else
		{
			refused.push_back(n);
			s.RemoveFromUnassigneds(n);
		}
	}		
}


void SequentialInsertionSBRP::InsertOld(Sol & s)
{
	s.Update();
	std::vector<Node*> nodes; //list of customers to insert
	std::vector<Node*> refused; //list of infeasible customers
	_insrmv.FillInsertionList(s, nodes);
	//printf("SeqInsertion %d\n", s.GetUnassignedCount());

	for(size_t i=0;i<nodes.size();i++)
	{
		Node * n = nodes[i];
		//printf("Node:%d\n", n->id);

		Move best;
		best.IsFeasible = false;
		best.DeltaCost = INFINITE;
		bool has_seen_empty_driver = false;
		for(int j = 0 ; j < s.GetDriverCount() ; j++)
		{
			Move m;
			if(s.GetRouteLength( s.GetDriver(j)) == 0 && has_seen_empty_driver) continue;
			
			_insrmv.InsertCost(s, n, s.GetDriver(j), m);
			//printf("Insert cost:%.2lf driver:%d Feasible:%d\n", m.DeltaCost, s.GetDriver(j)->id, m.IsFeasible );
			
			if(s.GetRouteLength( s.GetDriver(j)) == 0)
				has_seen_empty_driver = true;

			if(m.IsFeasible && m.DeltaCost < best.DeltaCost)
				best = m;
		}

		best.from = NULL;
		if(best.IsFeasible)
		{
			_insrmv.ApplyInsertMove(s, best);
			s.Update(best.to);
		}
		else
		{
			refused.push_back(n);
			s.RemoveFromUnassigneds(n);
		}
	}//end while


	for(size_t i = 0 ; i < refused.size() ; i++)
		s.AddToUnassigneds( refused[i] );
}