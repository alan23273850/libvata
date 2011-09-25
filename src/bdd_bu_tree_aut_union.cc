/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of union on BDD bottom-up tree automata.
 *
 *****************************************************************************/

// VATA headers
#include <vata/vata.hh>
#include <vata/bdd_bu_tree_aut_op.hh>

using VATA::BDDBottomUpTreeAut;
using VATA::Util::Convert;

// Standard library headers
#include <unordered_map>


BDDBottomUpTreeAut VATA::Union(const BDDBottomUpTreeAut& lhs,
	const BDDBottomUpTreeAut& rhs, AutBase::StateToStateMap* pTranslMapLhs,
	AutBase::StateToStateMap* pTranslMapRhs)
{
	// Assertions
	assert(lhs.isValid());
	assert(rhs.isValid());

	typedef BDDBottomUpTreeAut::StateType StateType;
	typedef BDDBottomUpTreeAut::StateTuple StateTuple;
	typedef BDDBottomUpTreeAut::StateSet StateSet;
	typedef BDDBottomUpTreeAut::TransMTBDD TransMTBDD;
	typedef VATA::AutBase::StateToStateMap StateToStateMap;
	typedef BDDBottomUpTreeAut::StateToStateTranslator StateToStateTranslator;


	BDDBottomUpTreeAut::UnionApplyFunctor unionFunc;

	if (lhs.GetTransTable() == rhs.GetTransTable())
	{	// in case the automata share their transition table
		BDDBottomUpTreeAut result = lhs;

		StateTuple tuple;
		const TransMTBDD& lhsMTBDD = lhs.GetMtbdd(tuple);
		const TransMTBDD& rhsMTBDD = rhs.GetMtbdd(tuple);

		result.SetMtbdd(tuple, unionFunc(lhsMTBDD, rhsMTBDD));

		assert(result.isValid());
		return result;
	}
	else
	{	// in case the automata have distinct transition tables
		StateToStateMap translMapLhs;
		if (pTranslMapLhs == nullptr)
		{	// copy states
			pTranslMapLhs = &translMapLhs;
		}

		StateToStateMap translMapRhs;
		if (pTranslMapRhs == nullptr)
		{
			pTranslMapRhs = &translMapRhs;
		}

		BDDBottomUpTreeAut result;

		StateType stateCnt = 0;
		auto translFunc = [&stateCnt](const StateType&){return stateCnt++;};

		StateToStateTranslator stateTransLhs(*pTranslMapLhs, translFunc);
		TransMTBDD lhsMtbdd = lhs.ReindexStates(result, stateTransLhs);
		for (const StateType& lhsFst : lhs.GetFinalStates())
		{
			result.SetStateFinal(stateTransLhs(lhsFst));
		}

		StateToStateTranslator stateTransRhs(*pTranslMapRhs, translFunc);
		TransMTBDD rhsMtbdd = rhs.ReindexStates(result, stateTransRhs);
		for (const StateType& rhsFst : rhs.GetFinalStates())
		{
			result.SetStateFinal(stateTransRhs(rhsFst));
		}

		result.SetMtbdd(StateTuple(), unionFunc(lhsMtbdd, rhsMtbdd));

		assert(result.isValid());
		return result;
	}
}
