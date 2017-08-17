#pragma once

#include "spectral/Spectrum.h"

namespace PR
{
	class PR_LIB_UTILS SpectralFile
	{
	public:
		SpectralFile();
		SpectralFile(uint32 width, uint32 height);
		SpectralFile(uint32 width, uint32 height, Spectrum* data, bool copy = false);
		SpectralFile(const SpectralFile& other);
		SpectralFile(SpectralFile&& other);
		virtual ~SpectralFile();

		SpectralFile& operator = (const SpectralFile& other);
		SpectralFile& operator = (SpectralFile&& other);

		void set(uint32 row, uint32 column, const Spectrum& spec);
		const Spectrum& at(uint32 row, uint32 column) const;

		void save(const std::string& path) const;
		static SpectralFile open(const std::string& path);

		inline uint32 width() const { return mData ? mData->Width : 0; }
		inline uint32 height() const { return mData ? mData->Height : 0; }
		inline Spectrum* ptr() const { return mData ? mData->Ptr : nullptr; }
		inline bool isValid() const { return mData != nullptr; }
	private:
		void deref();
		
		struct _Data
		{
			Spectrum* Ptr;
			uint32 Width;
			uint32 Height;
			bool External;
			uint32 Refs;
		}* mData;
	};
}
