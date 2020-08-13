#pragma once

#include <array>
#include <type_traits>

#include "PR_Config.h"

namespace PR {
template <size_t D, typename T, typename UniformT>
class NTreeBuilder;

/// n-dimensional tree with fixed topology. The tree itself can be built by the corresponding builder
template <size_t D, typename T, typename UniformT = float>
class PR_LIB_BASE NTree {
	friend class NTreeBuilder<D, T, UniformT>;

public:
	using ValueType	  = T;
	using UniformType = UniformT;
	using Dimension	  = std::integral_constant<size_t, D>;
	using NodeCount	  = std::integral_constant<size_t, (1 << D)>;
	using Builder	  = NTreeBuilder<D, T, UniformT>;
	template <typename T2>
	using Index		   = std::array<T2, D>;
	using LinearIndex  = size_t;
	using UniformIndex = Index<UniformT>; // All components are between 0 and 1

	static_assert(D > 0, "Dimension has to be greater than zero");
	static_assert(D < 32, "Dimension has to be less than 32");
	static_assert(std::is_copy_assignable<T>::value, "Given value type has to be copy assignable");
	static_assert(std::is_floating_point<UniformT>::value, "Expected uniform type to be a floating-point type");

	NTree()	 = default;
	~NTree() = default;

	/// Returns value at the bucket given by the uniform index
	inline T valueAt(const UniformIndex& idx) const;
	//inline void insertAt(const UniformIndex& idx, const T& value); /* TODO */

	/// Returns linear index of the bucket given by the uniform index
	inline LinearIndex indexAt(const UniformIndex& idx) const;
	/// Returns value at the bucket given by a linear index previously calculated by indexAt
	inline T valueAt(const LinearIndex& idx) const;
	/// Replaces value at the bucket given by a linear index previously calculated by indexAt
	inline void replaceAt(const LinearIndex& idx, const T& value);

	inline size_t totalNodeCount() const { return mLeafNodeCount + mBranchNodeCount; }
	inline size_t leafNodeCount() const { return mLeafNodeCount; }
	inline size_t branchNodeCount() const { return mBranchNodeCount; }
	inline size_t maxDepth() const { return mMaxDepth; }

private:
	inline void bake();

	class Node {
	public:
		Node()			= default;
		virtual ~Node() = default;

		virtual bool isLeaf() const = 0;
	};

	using NodePtr = std::unique_ptr<Node>;

	class LeafNode : public Node {
	public:
		inline LeafNode(const T& v)
			: Node()
			, mValue(v)
		{
		}
		virtual ~LeafNode() = default;

		bool isLeaf() const override { return true; }

		inline T value() const { return mValue; }
		inline T& value() { return mValue; }

	private:
		T mValue;
	};

	class BranchNode : public Node {
		friend class NTreeBuilder<D, T, UniformT>;

	public:
		inline BranchNode(const UniformIndex& pivot)
			: Node()
			, mPivot(pivot)
		{
		}
		virtual ~BranchNode() = default;

		bool isLeaf() const override { return false; }

		NodePtr node(size_t index) const { return mNodes[index]; }
		NodePtr& node(size_t index) { return mNodes[index]; }

		inline void calculateTotalNodeCount();
		inline size_t totalNodeCount() const { return mTotalNodeCount; }

		inline T valueAt(const UniformIndex& idx) const;
		inline LinearIndex indexAt(const UniformIndex& idx) const;

		inline T valueAt(const LinearIndex& idx) const;
		inline void replaceAt(const LinearIndex& idx, const T& value);

		inline void calculateStats(size_t depth, size_t& maxDepth, size_t& leafCount, size_t& branchCount);

	private:
		inline size_t childIndexAt(const UniformIndex& idx) const;
		inline size_t childIndexAt(const LinearIndex& idx, size_t& preNodeCount) const;
		inline LinearIndex linearIndexUntill(size_t childIndex) const;

		std::array<NodePtr, NodeCount::value> mNodes;
		size_t mTotalNodeCount = 0;
		UniformIndex mPivot;
	};

	NodePtr mRoot;
	size_t mLeafNodeCount	= 0;
	size_t mBranchNodeCount = 0;
	size_t mMaxDepth		= 0;
};

template <typename T>
using BinTree = NTree<1, T, float>;
template <typename T>
using QuadTree = NTree<2, T, float>;
template <typename T>
using OctTree = NTree<3, T, float>;
template <typename T>
using HexTree = NTree<4, T, float>;
} // namespace PR

#include "NTree.inl"