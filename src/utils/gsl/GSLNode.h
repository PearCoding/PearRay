#pragma once

#include "Config.h"

#ifdef PR_WITH_VIEWER
# include "PearMath.h"
#endif

#include <string>

namespace PRU
{
	class GSLMaterial;
	class PR_LIB_UTILS_INLINE GSLNode
	{
	public:
		inline GSLNode(GSLMaterial* mat) :
			mMaterial(mat)
#ifdef PR_WITH_VIEWER
			, mName(type()), mPosition(PM::pm_Zero())
#endif
		{
		}

#ifdef PR_WITH_VIEWER
		inline void setName(const std::string& name)
		{
			mName = name;
		}

		inline std::string name() const
		{
			return mName;
		}

		inline void setPosition(const PM::vec2& p)
		{
			mPosition = p;
		}

		inline PM::vec2 position() const
		{
			return mPosition;
		}
#endif

		inline GSLMaterial* material() const
		{
			return mMaterial;
		}

		virtual std::string type() const = 0;

	private:
		GSLMaterial* mMaterial;

#ifdef PR_WITH_VIEWER
		std::string mName;
		PM::vec2 mPosition;
#endif
	};
}