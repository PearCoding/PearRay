#include "IIntegrator.h"

namespace PR {

IIntegrator::IIntegrator(RenderContext* renderer)
	: mRenderer(renderer)
{
}

IIntegrator::~IIntegrator()
{
}

} // namespace PR
