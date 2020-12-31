#pragma once

#include "LightPathToken.h"
#include "math/Hash.h"
#include <unordered_map>
#include <unordered_set>

namespace PR {
namespace LPE {
struct Token {
	char Type;
	char Event;
	std::string Label;

	Token(char t, char e, const std::string& l = "")
		: Type(t)
		, Event(e)
		, Label(l)
	{
	}

	Token()
		: Type(0)
		, Event(0)
		, Label()
	{
	}

	inline bool isEmpty() const { return Type == 0; }
	inline bool operator==(const Token& other) const
	{
		return Type == other.Type && Event == other.Event && Label == other.Label;
	}
	inline bool operator!=(const Token& other) const
	{
		return !(*this == other);
	}

	inline bool match(ScatteringType t, ScatteringEvent e) const
	{
		return match(t) && match(e);
	}

	inline bool match(ScatteringType t) const
	{
		switch (Type) {
		case 'C':
			return t == ScatteringType::Camera;
		case 'E':
			return t == ScatteringType::Emissive;
		case 'B':
			return t == ScatteringType::Background;
		case 'L':
			return t == ScatteringType::Emissive || t == ScatteringType::Background;
		case 'R':
			return t == ScatteringType::Reflection;
		case 'T':
			return t == ScatteringType::Refraction;
		case '.':
			return t == ScatteringType::Reflection || t == ScatteringType::Refraction;
		default:
			return false;
		}
	}

	inline bool match(ScatteringEvent e) const
	{
		switch (Event) {
		case 'D':
			return e == ScatteringEvent::Diffuse;
		case 'S':
			return e == ScatteringEvent::Specular;
		case '.':
			return true;
		default:
			return false;
		}
	}
};

struct TokenHash {
	std::size_t operator()(Token const& k) const noexcept
	{
		std::size_t s = 0;
		hash_combine(s, k.Type);
		hash_combine(s, k.Event);
		hash_combine(s, k.Label);
		return s;
	}
};

typedef std::unordered_set<Token, TokenHash> TokenSet;

struct Transition {
	std::shared_ptr<class RegState> To;
	TokenSet Tokens;

	inline Transition(const Token& token, const std::shared_ptr<RegState>& to)
		: To(to)
		, Tokens({ token })
	{
	}

	inline Transition(const TokenSet& tokens, const std::shared_ptr<RegState>& to)
		: To(to)
		, Tokens(tokens)
	{
	}

	inline bool hasToken(const Token& token) const
	{
		bool found = Tokens.count(token) != 0;
		//return Negation ? !found : found;
		return found;
	}
};

class RegState {
public:
	typedef std::vector<Transition> TransitionMap;
	typedef std::unordered_set<std::shared_ptr<RegState>> StateSet;

	RegState()
		: mInitial(false)
		, mFinal(false)
	{
	}

	RegState(const StateSet& states)
		: mStates(states)
		, mInitial(false)
		, mFinal(false)
	{
		for (const auto& state : states) {
			if (state->mFinal) {
				mFinal = true;
				break;
			}
		}

		for (const auto& state : states) {
			if (state->mInitial) {
				mInitial = true;
				break;
			}
		}
	}

	inline void addTransition(const Token& token,
							  const std::shared_ptr<RegState>& state)
	{
		for (auto& s : mTransitions) {
			if (s.To == state) {
				s.Tokens.insert(token);
				return;
			}
		}

		mTransitions.emplace_back(token, state);
	}

	inline void addTransition(const TokenSet& tokens,
							  const std::shared_ptr<RegState>& state)
	{
		for (auto& s : mTransitions) {
			if (s.To == state) {
				s.Tokens.insert(tokens.begin(), tokens.end());
				return;
			}
		}

		mTransitions.emplace_back(tokens, state);
	}

	inline const TransitionMap& transitions() const { return mTransitions; }
	inline size_t transitionCount() const { return mTransitions.size(); }

	inline void removeTransitionsTo(const std::shared_ptr<RegState>& state)
	{
		for (auto it = mTransitions.begin(); it != mTransitions.end();) {
			if (it->To == state) {
				it = mTransitions.erase(it);
			} else {
				++it;
			}
		}
	}
	inline bool isFinal() const { return mFinal; }
	inline bool isInitial() const { return mInitial; }
	inline void makeFinal(bool f = true) { mFinal = f; }
	inline void makeInitial(bool f = true) { mInitial = f; }
	inline void flipType() { std::swap(mInitial, mFinal); }

	inline bool isDeadEnd() const
	{
		if (isFinal())
			return false;

		if (mTransitions.empty())
			return true;

		for (const auto& p : mTransitions) {
			if (p.To.get() != this)
				return false;
		}

		return true;
	}

	inline const StateSet& states() const { return mStates; }

private:
	TransitionMap mTransitions;
	StateSet mStates; // States whom it was constructed
	bool mInitial;
	bool mFinal;
};
} // namespace LPE
} // namespace PR