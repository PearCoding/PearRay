#pragma once

#include "Integrator.h"

namespace PR {
class PR_LIB OnePassIntegrator : public Integrator {
public:
	explicit OnePassIntegrator(RenderContext* renderer)
		: Integrator(renderer)
	{
	}

	void onStart() override;
	void onNextPass(uint32 i, bool& clean) override;
	void onEnd() override;

	void onPass(const RenderSession& session, uint32 i) override;

	bool needNextPass(uint32 i) const override;

	RenderStatus status() const;
};
}
