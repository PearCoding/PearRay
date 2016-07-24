#include "ConstVectorOutput.h"

#include "Logger.h"

using namespace PR;
namespace PRU
{
	ConstVectorShaderOutput::ConstVectorShaderOutput(const PM::vec& f) :
		VectorShaderOutput(), mValue(f)
	{
	}

	PM::vec ConstVectorShaderOutput::eval(const PR::SamplePoint& point)
	{
		return mValue;
	}
}