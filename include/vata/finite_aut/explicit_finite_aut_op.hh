/*****************************************************************************
 *	VATA Finite Automata Library
 *
 *	Copyright (c) 2013	Martin Hruska <xhrusk16@stud.fit.vutbr.cz>
 *
 *	Description:
 *	Header file for operations on finite automata.
 *
 *****************************************************************************/


#ifndef _VATA_EXPLICIT_FINITE_AUT_OP_HH_
#define _VATA_EXPLICIT_FINITE_AUT_OP_HH_

// VATA headers
#include <vata/vata.hh>
#include <vata/util/transl_strict.hh>
#include <vata/util/two_way_dict.hh>
#include <vata/util/binary_relation.hh>
#include <vata/util/transl_weak.hh>
#include <vata/explicit_lts.hh>

#include <vata/finite_aut/explicit_finite_aut.hh>
#include <vata/finite_aut/explicit_finite_isect.hh>
#include <vata/finite_aut/explicit_finite_unreach.hh>
#include <vata/finite_aut/explicit_finite_useless.hh>
#include <vata/finite_aut/explicit_finite_reverse.hh>
#include <vata/finite_aut/explicit_finite_compl.hh>
#include <vata/finite_aut/explicit_finite_candidate.hh>
#include <vata/finite_aut/explicit_finite_transl.hh>

#include <vata/finite_aut/explicit_finite_incl.hh>

#include <vata/finite_aut/explicit_finite_incl_fctor.hh>
#include <vata/finite_aut/explicit_finite_incl_fctor_opt.hh>

#include <vata/finite_aut/explicit_finite_congr_fctor.hh>
#include <vata/finite_aut/explicit_finite_congr_fctor_opt.hh>
#include <vata/finite_aut/explicit_finite_congr_equiv_fctor.hh>
#include <vata/finite_aut/explicit_finite_congr_fctor_cache_opt.hh>

#include <vata/finite_aut/util/map_to_list.hh>
#include <vata/finite_aut/util/macrostate_cache.hh>
#include <vata/finite_aut/util/congr_product.hh>

namespace VATA {
	template <class SymbolType,
		class Rel,
		class Index = Util::IdentityTranslator<AutBase::StateType>>
	ExplicitFiniteAut<SymbolType> CollapseStates(
			const ExplicitFiniteAut<SymbolType> &aut,
			const Rel & rel,
			const Index &index = Index()) {

		std::vector<size_t> representatives;

		// Creates vector, which contains equivalences classes of states of
		// aut automaton
		// If relation is identity, newly created automaton will be same
		rel.buildClasses(representatives);

		std::vector<AutBase::StateType> rebinded(representatives.size());

		// Transl will contain new numbers (indexes) for input states
		Util::RebindMap2(rebinded, representatives, index);
		ExplicitFiniteAut<SymbolType> res;
		aut.ReindexStates(res, rebinded);

		return res;
	}

	/******************************************************
	 * Functions prototypes
	 */

	template <class SymbolType>
	ExplicitFiniteAut<SymbolType> Reverse(
		const ExplicitFiniteAut<SymbolType> &aut,
		AutBase::ProductTranslMap* pTranslMap = nullptr);


	 template <class SymbolType, class Dict>
	 ExplicitFiniteAut<SymbolType> Complement(
			const ExplicitFiniteAut<SymbolType> &aut,
			const Dict &alphabet);


	template <class SymbolType>
	ExplicitFiniteAut<SymbolType> GetCandidateTree(
			const ExplicitFiniteAut<SymbolType>& aut);


	// Translator for simulation
	template <class SymbolType, class Index>
	ExplicitLTS Translate(const ExplicitFiniteAut<SymbolType>& aut,
		std::vector<std::vector<size_t>>& partition,
		Util::BinaryRelation& relation,
		const Index& stateIndex);
}
#endif
