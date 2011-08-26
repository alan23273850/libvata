/*****************************************************************************
 *  Vojnar's Army Tree Automata Library
 *
 *  Copyright (c) 2011  Ondra Lengal <ilengal@fit.vutbr.cz>
 *
 *  Description:
 *    My own implementation of multi-terminal binary decision diagrams (MTBDDs)
 *
 *****************************************************************************/

#ifndef _VATA_ONDRIKS_MTBDD_HH_
#define _VATA_ONDRIKS_MTBDD_HH_

// VATA headers
#include	<vata/vata.hh>
#include	<vata/mtbdd/mtbdd_node.hh>
#include	<vata/mtbdd/var_asgn.hh>
#include	<vata/util/triple.hh>

// Standard library headers
#include	<cassert>
#include	<stdint.h>
#include	<stdexcept>
#include	<vector>
#include  <memory>
#include  <unordered_map>

// Boost library headers
#include <boost/functional/hash.hpp>

namespace VATA
{
	namespace MTBDDPkg
	{
		template <
			typename Data
		>
		class OndriksMTBDD;

		template <class, typename, typename>
		class Apply1Functor;

		template <class, typename, typename, typename>
		class Apply2Functor;

		template <class, typename, typename, typename, typename>
		class Apply3Functor;

		template <class, typename>
		class VoidApply1Functor;

		template <class, typename, typename>
		class VoidApply2Functor;

		template <class, typename, typename, typename>
		class VoidApply3Functor;
	}
}


/**
 * @brief   Class representing MTBDD
 *
 * This class represents a single multi-terminal binary decision diagram
 * (MTBDD).
 *
 * @tparam  Data  The type of leaves
 */
template <
	typename Data
>
class VATA::MTBDDPkg::OndriksMTBDD
{
	template <class, typename, typename>
	friend class Apply1Functor;

	template <class, typename, typename, typename>
	friend class Apply2Functor;

	template <class, typename, typename, typename, typename>
	friend class Apply3Functor;

	template <class, typename>
	friend class VoidApply1Functor;

	template <class, typename, typename>
	friend class VoidApply2Functor;

	template <class, typename, typename, typename>
	friend class VoidApply3Functor;

public:   // public data types

	typedef Data DataType;

private:  // private data types

	typedef MTBDDNodePtr<DataType> NodePtrType;

public:   // public data types

	typedef typename NodePtrType::VarType VarType;
	typedef std::vector<VarType> PermutationTable;
	typedef std::shared_ptr<PermutationTable> PermutationTablePtr;

private:  // private data types

	typedef VATA::Util::Triple<NodePtrType, NodePtrType, VarType>
		InternalAddressType;

	typedef std::unordered_map<InternalAddressType, NodePtrType,
		boost::hash<InternalAddressType>> InternalCacheType;

	typedef std::unordered_map<DataType, NodePtrType,
		boost::hash<DataType>> LeafCacheType;

private:  // private data members

	NodePtrType root_;

	DataType defaultValue_;

	static LeafCacheType leafCache_;
	static InternalCacheType internalCache_;


private:  // private methods


	/**
	 * @brief  Function for constructing an MTBDD
	 *
	 * This function is used for constructing a new MTBDD, such that the
	 * variable assignment @p asgn is set to @p value and all other assignments
	 * are set to @p defaultValue.
	 *
	 * @param  asgn          Variable assignment to be set to @p value
	 * @param  value         Value to be set for assignment @p asgn
	 * @param  defaultValue  Value to be set for all assignments other than @p
	 *                       asgn
	 *
	 * @return  The constructed MTBDD
	 */
	static NodePtrType constructMTBDD(const VarAsgn& asgn,
		const DataType& value, const DataType& defaultValue)
	{
		// the leaf with the desired value
		NodePtrType leaf = spawnLeaf(value);

		if (value == defaultValue)
		{	// in case an MTBDD with a single leaf is desired
			IncrementRefCnt(leaf);
			return leaf;
		}

		// the sink leaf
		NodePtrType sink = spawnLeaf(defaultValue);

		// working node
		NodePtrType node = leaf;

		for (size_t i = 0; i < asgn.length(); ++i)
		{	// construct the MTBDD according to the variable ordering
			VarType var =	i;
			if (asgn.GetIthVariableValue(var) == VarAsgn::ONE)
			{	// in case the variable is 1
				node = spawnInternal(sink, node, var);
			}
			else if (asgn.GetIthVariableValue(var) == VarAsgn::ZERO)
			{	// in case the variable is 0
				node = spawnInternal(node, sink, var);
			}
			// otherwise don't care about the variable
		}

		if (node == leaf)
		{	// in case the MTBDD is with a single leaf
			assert(IsLeaf(leaf));

			if (GetLeafRefCnt(sink) == 0)
			{	// in case there is no one pointing to the sink
				disposeOfLeafNode(sink);
			}
		}

		IncrementRefCnt(node);
		return node;
	}

	OndriksMTBDD(NodePtrType root, const DataType& defaultValue) :
		root_(root),
		defaultValue_(defaultValue)
	{
		// Assertions
		assert(!IsNull(root_));
	}


	NodePtrType getRoot() const
	{
		return root_;
	}

	static void disposeOfLeafNode(NodePtrType node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsLeaf(node));

		if (leafCache_.erase(GetDataFromLeaf(node)) != 1)
		{	// in case the leaf was not cached
			assert(false);     // fail gracefully
		}

		DeleteLeafNode(node);
	}

	static void disposeOfInternalNode(NodePtrType node)
	{
		// Assertions
		assert(!IsNull(node));
		assert(IsInternal(node));

		InternalAddressType addr(GetLowFromInternal(node),
			GetHighFromInternal(node), GetVarFromInternal(node));
		if (internalCache_.erase(addr) != 1)
		{	// in case the internal was not cached
			assert(false);   // fail gracefully
		}

		recursivelyDeleteMTBDDNode(GetLowFromInternal(node));
		recursivelyDeleteMTBDDNode(GetHighFromInternal(node));

		DeleteInternalNode(node);
	}

	static void recursivelyDeleteMTBDDNode(NodePtrType node)
	{
		// Assertions
		assert(!IsNull(node));

		if (IsLeaf(node))
		{	// for leaves
			if (DecrementLeafRefCnt(node) == 0)
			{	// this reference to node is the last
				disposeOfLeafNode(node);
			}
		}
		else
		{	// for internal nodes
			assert(IsInternal(node));

			if (DecrementInternalRefCnt(node) == 0)
			{	// this reference to node is the last
				disposeOfInternalNode(node);
			}
		}
	}

	inline void deleteMTBDD()
	{
		if (!IsNull(root_))
		{
			recursivelyDeleteMTBDDNode(root_);
			root_ = 0;
		}
	}

	static inline NodePtrType spawnLeaf(const DataType& data)
	{
		NodePtrType result = 0;

		typename LeafCacheType::const_iterator itLC;
		if ((itLC = leafCache_.find(data)) != leafCache_.end())
		{	// in case given leaf is already cached
			result = itLC->second;
		}
		else
		{	// if the leaf doesn't exist
			result = CreateLeaf(data);
			leafCache_.insert(std::make_pair(data, result));
		}

		assert(!IsNull(result));
		return result;
	}

	static inline NodePtrType spawnInternal(
		NodePtrType low, NodePtrType high, const VarType& var)
	{
		NodePtrType result = 0;

		InternalAddressType addr(low, high, var);
		typename InternalCacheType::const_iterator itIC;
		if ((itIC = internalCache_.find(addr)) != internalCache_.end())
		{	// in case given internal is already cached
			result = itIC->second;
		}
		else
		{	// if the internal doesn't exist
			result = CreateInternal(low, high, var);
			IncrementRefCnt(low);
			IncrementRefCnt(high);
			internalCache_.insert(std::make_pair(addr, result));
		}

		assert(!IsNull(result));
		return result;
	}

public:   // public methods


	/**
	 * @brief  Constructor with given variable ordering
	 *
	 * This constructor creates a new MTBDD , such that the variable assignment
	 * @p asgn is set to @p value and all other assignments are set to @p
	 * defaultValue.
	 *
	 * @param  asgn          Variable assignment to be set to @p value
	 * @param  value         Value to be set for assignment @p asgn
	 * @param  defaultValue  Value to be set for all assignments other than @p
	 *                       asgn
	 */
	OndriksMTBDD(const VarAsgn& asgn, const DataType& value,
		const DataType& defaultValue) :
		root_(static_cast<uintptr_t>(0)),
		defaultValue_(defaultValue)
	{
		// create the MTBDD
		root_ = constructMTBDD(asgn, value, defaultValue_);
	}

	OndriksMTBDD(const OndriksMTBDD& mtbdd)
		: root_(mtbdd.root_),
			defaultValue_(mtbdd.defaultValue_)
	{
		// Assertions
		assert(!IsNull(root_));

		IncrementRefCnt(root_);
	}

	explicit OndriksMTBDD(const DataType& value)
		: root_(spawnLeaf(value)),
			defaultValue_(value)
	{
		// Assertions
		assert(!IsNull(root_));

		// TODO: do something about variable ordering

		IncrementRefCnt(root_);
	}

	OndriksMTBDD& operator=(const OndriksMTBDD& mtbdd)
	{
		// Assertions
		assert(!IsNull(root_));

		if (&mtbdd == this)
			return *this;

		deleteMTBDD();

		root_ = mtbdd.root_;
		IncrementRefCnt(root_);

		defaultValue_ = mtbdd.defaultValue_;

		return *this;
	}

	inline const DataType& GetDefaultValue() const
	{
		return defaultValue_;
	}

	/**
	 * @brief  Returns value for assignment
	 *
	 * Thsi method returns a value in the MTBDD that corresponds to given
	 * variable assignment @p asgn.
	 *
	 * @note  If there are multiple values that correspond to given variable
	 * assignment (e.g., because the assignment is very nondeterministic), an
	 * arbitrary value is returned.
	 *
	 * @param  asgn  Variable assignment
	 *
	 * @return  Value corresponding to variable assignment @p asgn
	 */
	const DataType& GetValue(const VarAsgn& asgn) const
	{
		NodePtrType node = root_;

		while (!IsLeaf(node))
		{	// try to proceed according to the assignment
			const VarType& var = GetVarFromInternal(node);

			if (asgn.GetIthVariableValue(var) == VarAsgn::ONE)
			{	// if one
				node = GetHighFromInternal(node);
			}
			else
			{	// if zero or don't care
				node = GetLowFromInternal(node);
			}
		}

		return GetDataFromLeaf(node);
	}

	~OndriksMTBDD()
	{
		deleteMTBDD();
	}
};

template <typename Data>
typename VATA::MTBDDPkg::OndriksMTBDD<Data>::LeafCacheType
	VATA::MTBDDPkg::OndriksMTBDD<Data>::leafCache_;

template <typename Data>
typename VATA::MTBDDPkg::OndriksMTBDD<Data>::InternalCacheType
	VATA::MTBDDPkg::OndriksMTBDD<Data>::internalCache_;

#endif
