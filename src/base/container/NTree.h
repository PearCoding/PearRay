#pragma once

#include <array>
#include <type_traits>

#include "PR_Config.h"

namespace PR {
template <size_t D, typename T, typename UniformT>
class NTreeStackBuilder;

/// n-dimensional tree with fixed topology and fixed pivots. The tree itself can be built by the corresponding builder
template <size_t D, typename T, typename UniformT = float>
class PR_LIB_BASE NTree {
	friend class NTreeStackBuilder<D, T, UniformT>;

public:
	using ValueType	   = T;
	using UniformType  = UniformT;
	using Dimension	   = std::integral_constant<size_t, D>;
	using NodeCount	   = std::integral_constant<size_t, (1 << D)>;
	using StackBuilder = NTreeStackBuilder<D, T, UniformT>;

	template <typename T2>
	using Index		   = Eigen::Array<T2, D, 1, Eigen::DontAlign>;
	using UniformIndex = Index<UniformT>; // All components are between 0 and 1

	static_assert(D > 0, "Dimension has to be greater than zero");
	static_assert(D < 32, "Dimension has to be less than 32");
	static_assert(std::is_copy_assignable<T>::value, "Given value type has to be copy assignable");
	static_assert(std::is_floating_point<UniformT>::value, "Expected uniform type to be a floating-point type");

	NTree()	 = default;
	~NTree() = default;

	/// Returns value at the bucket given by the uniform index
	inline T valueAt(const UniformIndex& idx) const;
	inline void insertAt(const UniformIndex& idx, const T& value);

	inline T nearestValueAt(const UniformIndex& idx, UniformT maxRadius = std::numeric_limits<UniformT>::max()) const;

	inline size_t totalNodeCount() const;
	inline size_t leafNodeCount() const;
	inline size_t branchNodeCount() const;
	inline size_t maxDepth() const;

	inline void clear();

	template <typename Func>
	inline void visit(const Func& func);

	template <typename Func>
	inline void visit(const Func& func) const;

	template <typename Func>
	inline void enumerate(const Func& func) const;

private:
	class Node {
	public:
		Node()			= default;
		virtual ~Node() = default;

		virtual bool isLeaf() const = 0;
	};

	using NodePtr = std::unique_ptr<Node>;

	class LeafNode : public Node {
	public:
		inline LeafNode(const UniformIndex& idx, const T& v)
			: Node()
			, mValue(v)
			, mIndex(idx)
		{
		}
		virtual ~LeafNode() = default;

		bool isLeaf() const override { return true; }

		inline T value() const { return mValue; }
		inline T& value() { return mValue; }

		inline UniformIndex index() const { return mIndex; }
		inline UniformIndex& index() { return mIndex; }

	private:
		T mValue;
		UniformIndex mIndex;
	};

	class BranchNode : public Node {
		friend class NTreeStackBuilder<D, T, UniformT>;

	public:
		inline BranchNode(const UniformIndex& pivot)
			: Node()
			, mPivot(pivot)
		{
		}
		virtual ~BranchNode() = default;

		bool isLeaf() const override { return false; }

		inline NodePtr node(size_t index) const { return mNodes[index]; }
		inline NodePtr& node(size_t index) { return mNodes[index]; }

		inline UniformIndex pivot() const { return mPivot; }

		inline T valueAt(const UniformIndex& idx) const;
		inline void insertAt(const UniformIndex& idx, const T& value, const UniformIndex& low, const UniformIndex& high);
		inline T nearestValueAt(const UniformIndex& idx, UniformT& maxRadius) const;

		inline size_t leafNodeCount() const;
		inline size_t branchNodeCount() const;
		inline size_t maxDepth() const;

		template <typename Func>
		inline void visit(const Func& func);

		template <typename Func>
		inline void visit(const Func& func) const;

		template <typename Func>
		inline void enumerate(const Func& func) const;

	private:
		inline size_t childIndexAt(const UniformIndex& idx) const;
		inline void adaptFor(const UniformIndex& idx, UniformIndex& low, UniformIndex& high) const;

		std::array<NodePtr, NodeCount::value> mNodes;
		UniformIndex mPivot;
	};

	NodePtr mRoot;
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