#include "TevObserver.h"
#include "Logger.h"
#include "ProgramSettings.h"
#include "buffer/Feedback.h"
#include "buffer/FrameBufferSystem.h"
#include "network/Socket.h"
#include "renderer/RenderContext.h"
#include "serialization/BufferedNetworkSerializer.h"
#include "spectral/RGBConverter.h"

#include <thread>

namespace PR {
static const char* IMAGE_NAME								= "PearRay";
static const uint32 IMAGE_NAME_SIZE							= strlen(IMAGE_NAME) + 1;
constexpr size_t MAX_CHANNEL_COUNT_RGB						= 3;
constexpr size_t MAX_CHANNEL_COUNT_F						= 1;
static const char* CHANNEL_NAMES_RGB[MAX_CHANNEL_COUNT_RGB] = { "R", "G", "B" };
static const char* CHANNEL_NAMES_VAR[MAX_CHANNEL_COUNT_RGB] = { "Variance.R", "Variance.G", "Variance.B" };
static const char* CHANNEL_NAMES_WEI[MAX_CHANNEL_COUNT_F]	= { "Weight" };
static const char* CHANNEL_NAMES_FDB[MAX_CHANNEL_COUNT_RGB] = { "Feedback.R", "Feedback.G", "Feedback.B" };
static const uint32 CREATE_MESSAGE_HEADER_SIZE				= 4 + 1 + IMAGE_NAME_SIZE + 1 + 4 + 4 + 4;
static const uint32 CLOSE_MESSAGE_SIZE						= 4 + 1 + IMAGE_NAME_SIZE;
static const uint32 UPDATE_MESSAGE_HEADER_SIZE				= 4 + 1 + IMAGE_NAME_SIZE + 1 + 4 + 4 + 4 + 4;
constexpr uint32 UPDATE_TILE_SIZE							= 64;

struct TevChannelInfo {
	const char* Name;
	size_t UpdateMessageSize;
};

class TevConnection {
public:
	Socket Con;
	BufferedNetworkSerializer Out;
	std::vector<float> Data;

	size_t CreateMessageSize;
	std::vector<TevChannelInfo> ChannelInfo;
	size_t MaxUpdateMessageSize;

	TevConnection(const std::string& ip, uint16 port)
		: CreateMessageSize(0)
	{
		Con.connect(port, ip);
		Out.setSocket(&Con, false);
	}
};

TevObserver::TevObserver()
	: mRenderContext(nullptr)
	, mUpdateCycleSeconds(0)
	, mDisplayVariance(false)
	, mDisplayWeight(false)
	, mDisplayFeedback(false)
{
}

TevObserver::~TevObserver()
{
}

void TevObserver::begin(RenderContext* renderContext, const ProgramSettings& settings)
{
	PR_ASSERT(renderContext, "Invalid render context");

	mRenderContext		= renderContext;
	mConnection			= std::make_unique<TevConnection>(settings.TevIp, settings.TevPort);
	mUpdateCycleSeconds = settings.TevUpdate;
	mDisplayVariance	= settings.TevVariance;
	mDisplayWeight		= settings.TevWeight;
	mDisplayFeedback	= settings.TevFeedback;
	mLastUpdate			= std::chrono::high_resolution_clock::now();

	if (mConnection->Con.isOpen()) {
		calculateProtocolCache();
		closeImageProtocol(); // Make sure a potentially open file is closed
		createImageProtocol();
	}
}

void TevObserver::end()
{
	if (mConnection->Con.isOpen())
		updateImageProtocol();

	mConnection.reset();
}

void TevObserver::update(const UpdateInfo& info)
{
	PR_UNUSED(info);

	if (!mConnection->Con.isOpen())
		return;

	auto now	  = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - mLastUpdate);

	if ((uint64)duration.count() < mUpdateCycleSeconds)
		return;

	updateImageProtocol();

	mLastUpdate = now;
}

void TevObserver::onIteration(const UpdateInfo&)
{
	// Nothing
}

void TevObserver::calculateProtocolCache()
{
	mConnection->MaxUpdateMessageSize = 0;
	size_t full_channel_name_size	  = 0;

	for (size_t i = 0; i < MAX_CHANNEL_COUNT_RGB; i++) {
		size_t channel_name_size = strlen(CHANNEL_NAMES_RGB[i]) + 1;
		full_channel_name_size += channel_name_size;
		mConnection->ChannelInfo.push_back(TevChannelInfo{ CHANNEL_NAMES_RGB[i], UPDATE_MESSAGE_HEADER_SIZE + channel_name_size });
		mConnection->MaxUpdateMessageSize = std::max(mConnection->MaxUpdateMessageSize, UPDATE_MESSAGE_HEADER_SIZE + channel_name_size);
	}

	if (mDisplayVariance) {
		for (size_t i = 0; i < MAX_CHANNEL_COUNT_RGB; i++) {
			size_t channel_name_size = strlen(CHANNEL_NAMES_VAR[i]) + 1;
			full_channel_name_size += channel_name_size;
			mConnection->ChannelInfo.push_back(TevChannelInfo{ CHANNEL_NAMES_VAR[i], UPDATE_MESSAGE_HEADER_SIZE + channel_name_size });
			mConnection->MaxUpdateMessageSize = std::max(mConnection->MaxUpdateMessageSize, UPDATE_MESSAGE_HEADER_SIZE + channel_name_size);
		}
	}

	if (mDisplayWeight) {
		for (size_t i = 0; i < MAX_CHANNEL_COUNT_F; i++) {
			size_t channel_name_size = strlen(CHANNEL_NAMES_WEI[i]) + 1;
			full_channel_name_size += channel_name_size;
			mConnection->ChannelInfo.push_back(TevChannelInfo{ CHANNEL_NAMES_WEI[i], UPDATE_MESSAGE_HEADER_SIZE + channel_name_size });
			mConnection->MaxUpdateMessageSize = std::max(mConnection->MaxUpdateMessageSize, UPDATE_MESSAGE_HEADER_SIZE + channel_name_size);
		}
	}

	if (mDisplayFeedback) {
		for (size_t i = 0; i < MAX_CHANNEL_COUNT_RGB; i++) {
			size_t channel_name_size = strlen(CHANNEL_NAMES_FDB[i]) + 1;
			full_channel_name_size += channel_name_size;
			mConnection->ChannelInfo.push_back(TevChannelInfo{ CHANNEL_NAMES_FDB[i], UPDATE_MESSAGE_HEADER_SIZE + channel_name_size });
			mConnection->MaxUpdateMessageSize = std::max(mConnection->MaxUpdateMessageSize, UPDATE_MESSAGE_HEADER_SIZE + channel_name_size);
		}
	}

	mConnection->CreateMessageSize = CREATE_MESSAGE_HEADER_SIZE + full_channel_name_size;
}

// uint32 			MessageSize
// uint8 			Type = 4
// String 			Name
// bool/uint8 		GrabFocus
// int32 			Width
// int32 			Height
// int32 			Channels
// Channels*String 	ChannelNames
void TevObserver::createImageProtocol()
{
	auto channel = mRenderContext->output()->data().getInternalChannel_Spectral(AOV_Output);
	PR_ASSERT(channel->channels() == 3,
			  "Expect spectral channel to have 3 channels");

	mConnection->Data.resize(UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * mConnection->ChannelInfo.size()); // Make sure update calls have space
	mConnection->Out.resize(mConnection->MaxUpdateMessageSize + sizeof(float) * UPDATE_TILE_SIZE * UPDATE_TILE_SIZE);

	mConnection->Out.write((uint32)mConnection->CreateMessageSize);
	mConnection->Out.write((uint8)4);
	mConnection->Out.write((uint8)1);
	mConnection->Out.write(IMAGE_NAME);
	mConnection->Out.write((uint32)channel->width());
	mConnection->Out.write((uint32)channel->height());
	mConnection->Out.write((uint32)mConnection->ChannelInfo.size());

	for (const auto& channel : mConnection->ChannelInfo)
		mConnection->Out.write(channel.Name);

	mConnection->Out.flush();
}

// uint32 		MessageSize
// uint8 		Type = 2
// String 		Name
void TevObserver::closeImageProtocol()
{
	mConnection->Out.resize(CLOSE_MESSAGE_SIZE);

	mConnection->Out.write((uint32)CLOSE_MESSAGE_SIZE);
	mConnection->Out.write((uint8)2);
	mConnection->Out.write(IMAGE_NAME);

	//PR_ASSERT(mConnection->Out.currentUsed() == CLOSE_MESSAGE_SIZE, "Invalid package size");
	mConnection->Out.flush();
}

// uint32 		MessageSize
// uint8 		Type = 3
// bool/uint8 	GrabFocus
// String 		Name
// string 		Channel
// int32 		X = 0
// int32 		Y = 0
// int32 		Width
// int32 		Height
// float*W*H	Data
void TevObserver::updateImageProtocol()
{
	const auto channel		 = mRenderContext->output()->data().getInternalChannel_Spectral(AOV_Output);
	const auto blend_channel = mRenderContext->output()->data().getInternalChannel_1D(AOV_PixelWeight);
	const auto var_channel	 = mRenderContext->output()->data().getInternalChannel_Spectral(AOV_OnlineVariance);
	const auto fdb_channel	 = mRenderContext->output()->data().getInternalChannel_Counter(AOV_Feedback);

	PR_ASSERT(channel->channels() == 3, "Expect spectral channel to have 3 channels");

	const size_t tx = channel->width() / UPDATE_TILE_SIZE + 1;
	const size_t ty = channel->height() / UPDATE_TILE_SIZE + 1;

	const bool monotonic = mRenderContext->settings().spectralMono;

	// TODO: Better way?
	for (size_t i = 0; i < ty; ++i) {
		size_t sy = i * UPDATE_TILE_SIZE;
		size_t ey = std::min<size_t>(channel->height(), (i + 1) * UPDATE_TILE_SIZE);
		size_t h  = ey - sy;

		if (ey <= sy)
			continue;

		for (uint32 j = 0; j < tx; ++j) {
			size_t sx = j * UPDATE_TILE_SIZE;
			size_t ex = std::min<size_t>(channel->width(), (j + 1) * UPDATE_TILE_SIZE);
			size_t w  = ex - sx;

			if (ex <= sx)
				continue;

			// Copy to seperate channels and map to sRGB
			if (!monotonic) {
				PR_OPT_LOOP
				for (size_t iy = 0; iy < h; ++iy) {
					for (size_t ix = 0; ix < w; ++ix) {
						const auto p = Point2i(sx + ix, sy + iy);

						const float blendWeight = blend_channel->getFragment(p, 0);
						const float blendFactor = blendWeight <= PR_EPSILON ? 1.0f : 1.0f / blendWeight;

						const float x = channel->getFragment(p, 0);
						const float y = channel->getFragment(p, 1);
						const float z = channel->getFragment(p, 2);

						float r, g, b;
						RGBConverter::fromXYZ(x, y, z, r, g, b);
						mConnection->Data[UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * 0 + iy * w + ix] = r * blendFactor;
						mConnection->Data[UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * 1 + iy * w + ix] = g * blendFactor;
						mConnection->Data[UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * 2 + iy * w + ix] = b * blendFactor;
					}
				}
			} else {
				PR_OPT_LOOP
				for (size_t iy = 0; iy < h; ++iy) {
					for (size_t ix = 0; ix < w; ++ix) {
						const auto p			= Point2i(sx + ix, sy + iy);
						const float blendWeight = blend_channel->getFragment(p, 0);
						const float blendFactor = blendWeight <= PR_EPSILON ? 1.0f : 1.0f / blendWeight;
						for (size_t k = 0; k < 3; ++k)
							mConnection->Data[UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * k + iy * w + ix] = blendFactor * channel->getFragment(p, k);
					}
				}
			}

			size_t delta = 3;
			if (mDisplayVariance) {
				PR_OPT_LOOP
				for (size_t iy = 0; iy < h; ++iy) {
					for (size_t ix = 0; ix < w; ++ix) {
						const auto p			= Point2i(sx + ix, sy + iy);
						const float blendWeight = blend_channel->getFragment(p, 0);
						const float blendFactor = blendWeight <= PR_EPSILON ? 1.0f : 1.0f / blendWeight;
						for (size_t k = 0; k < 3; ++k)
							mConnection->Data[UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * (k + delta) + iy * w + ix] = blendFactor * var_channel->getFragment(p, k);
					}
				}
				delta += 3;
			}

			if (mDisplayWeight) {
				PR_OPT_LOOP
				for (size_t iy = 0; iy < h; ++iy) {
					for (size_t ix = 0; ix < w; ++ix) {
						const auto p			= Point2i(sx + ix, sy + iy);
						const float blendWeight = blend_channel->getFragment(p, 0);

						mConnection->Data[UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * (0 + delta) + iy * w + ix] = blendWeight;
					}
				}
				delta += 1;
			}

			if (mDisplayFeedback) {
				PR_OPT_LOOP
				for (size_t iy = 0; iy < h; ++iy) {
					for (size_t ix = 0; ix < w; ++ix) {
						const auto p   = Point2i(sx + ix, sy + iy);
						const auto fdb = fdb_channel->getFragment(p, 0);

						float r = 0;
						if (fdb & OF_NaN)
							r = 1.0f;
						else if (fdb & OF_Infinite)
							r = 0.5f;
						else if (fdb & OF_Negative)
							r = 0.25f;

						float g = 0;
						if (fdb & OF_MissingMaterial)
							g = 1.0f;

						float b = 0;
						if (fdb & OF_MissingEmission)
							b = 1.0f;

						mConnection->Data[UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * (0 + delta) + iy * w + ix] = r;
						mConnection->Data[UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * (1 + delta) + iy * w + ix] = g;
						mConnection->Data[UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * (2 + delta) + iy * w + ix] = b;
					}
				}
				delta += 3;
			}

			for (size_t c = 0; c < mConnection->ChannelInfo.size(); ++c) {
				const float* data		 = mConnection->Data.data() + UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * c;
				const uint32 messageSize = mConnection->ChannelInfo[c].UpdateMessageSize + sizeof(float) * h * w;

				mConnection->Out.write((uint32)messageSize);
				mConnection->Out.write((uint8)3);
				mConnection->Out.write((uint8)0);
				mConnection->Out.write(IMAGE_NAME);
				mConnection->Out.write(mConnection->ChannelInfo[c].Name);
				mConnection->Out.write((uint32)sx);
				mConnection->Out.write((uint32)sy);
				mConnection->Out.write((uint32)w);
				mConnection->Out.write((uint32)h);
				mConnection->Out.writeRaw(reinterpret_cast<const uint8*>(data), h * w * sizeof(float));

				if (mConnection->Con.isOpen()) {
					//PR_ASSERT(mConnection->Out.currentUsed() == messageSize, "Invalid package size");
					mConnection->Out.flush();
				} else {
					return;
				}
			}
		}
	}
}
} // namespace PR