#include "TevObserver.h"
#include "Logger.h"
#include "ProgramSettings.h"
#include "buffer/FrameBufferSystem.h"
#include "network/Socket.h"
#include "renderer/RenderContext.h"
#include "serialization/BufferedNetworkSerializer.h"
#include "spectral/RGBConverter.h"

#include <thread>

namespace PR {
static const char* IMAGE_NAME					= "PearRay";
constexpr uint32 IMAGE_NAME_SIZE				= 8;
constexpr uint32 CHANNEL_COUNT					= 3; // Only triplets supported yet
static const char* CHANNEL_NAMES[CHANNEL_COUNT] = { "R", "G", "B" };
constexpr uint32 CHANNEL_NAME_SIZE				= 2;
constexpr uint32 CREATE_MESSAGE_SIZE			= 4 + 1 + IMAGE_NAME_SIZE + 1 + 4 + 4 + 4 + 3 * CHANNEL_NAME_SIZE;
constexpr uint32 UPDATE_MESSAGE_HEADER_SIZE		= 4 + 1 + IMAGE_NAME_SIZE + 1 + 4 + 4 + 4 + 4 + CHANNEL_NAME_SIZE;
constexpr uint32 UPDATE_TILE_SIZE				= 64;

class TevConnection {
public:
	Socket Con;
	BufferedNetworkSerializer Out;
	std::vector<float> Data;

	TevConnection(const std::string& ip, uint16 port)
	{
		Con.connect(port, ip);
		Out.setSocket(&Con, false);
	}
};

TevObserver::TevObserver()
	: mRenderContext(nullptr)
	, mUpdateCycleSeconds(0)
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
	mLastUpdate			= std::chrono::high_resolution_clock::now();
	mMaxIterationCount	= mRenderContext->maxIterationCount();

	if (mConnection->Con.isOpen())
		createImageProtocol();
}

void TevObserver::end()
{
	if (mConnection->Con.isOpen())
		updateImageProtocol(1);

	mConnection.reset();
}

void TevObserver::update(const UpdateInfo& info)
{
	if (!mConnection->Con.isOpen())
		return;

	auto now	  = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - mLastUpdate);

	if ((uint64)duration.count() < mUpdateCycleSeconds)
		return;

	updateImageProtocol(mMaxIterationCount / ((float)info.CurrentIteration));

	mLastUpdate = now;
}

void TevObserver::onIteration(const UpdateInfo&)
{
	// Nothing
}

// uint32 		MessageSize
// uint8 		Type = 4
// String 		Name
// bool/uint8 	GrabFocus
// int32 		Width
// int32 		Height
// int32 		Channels = 3
// 3*String 	ChannelNames = R,G,B
void TevObserver::createImageProtocol()
{
	auto channel = mRenderContext->output()->data().getInternalChannel_Spectral();
	PR_ASSERT(channel->channels() == CHANNEL_COUNT,
			  "Expect spectral channel to have 3 channels");

	mConnection->Data.resize(UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * CHANNEL_COUNT); // Make sure update calls have space
	mConnection->Out.resize(UPDATE_MESSAGE_HEADER_SIZE + sizeof(float) * UPDATE_TILE_SIZE * UPDATE_TILE_SIZE);

	mConnection->Out.write((uint32)CREATE_MESSAGE_SIZE);
	mConnection->Out.write((uint8)4);
	mConnection->Out.write((uint8)1);
	mConnection->Out.write(IMAGE_NAME);
	mConnection->Out.write((uint32)channel->width());
	mConnection->Out.write((uint32)channel->height());
	mConnection->Out.write((uint32)CHANNEL_COUNT);
	for (uint32 i = 0; i < CHANNEL_COUNT; i++)
		mConnection->Out.write(CHANNEL_NAMES[i]);

	PR_ASSERT(mConnection->Out.currentUsed() == CREATE_MESSAGE_SIZE, "Invalid package size");
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
void TevObserver::updateImageProtocol(float scale)
{
	auto channel = mRenderContext->output()->data().getInternalChannel_Spectral();
	PR_ASSERT(channel->channels() == CHANNEL_COUNT,
			  "Expect spectral channel to have 3 channels");

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
						const auto p  = Point2i(sx + ix, sy + iy);
						const float x = scale * channel->getFragment(p, 0);
						const float y = scale * channel->getFragment(p, 1);
						const float z = scale * channel->getFragment(p, 2);
						float r, g, b;
						RGBConverter::fromXYZ(x, y, z, r, g, b);
						mConnection->Data[UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * 0 + iy * w + ix] = r;
						mConnection->Data[UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * 1 + iy * w + ix] = g;
						mConnection->Data[UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * 2 + iy * w + ix] = b;
					}
				}
			} else {
				PR_OPT_LOOP
				for (size_t iy = 0; iy < h; ++iy) {
					for (size_t ix = 0; ix < w; ++ix) {
						const auto p = Point2i(sx + ix, sy + iy);
						for (size_t k = 0; k < CHANNEL_COUNT; ++k)
							mConnection->Data[UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * k + iy * w + ix] = scale * channel->getFragment(p, k);
					}
				}
			}

			for (uint32 c = 0; c < CHANNEL_COUNT; ++c) {
				const float* data		 = mConnection->Data.data() + UPDATE_TILE_SIZE * UPDATE_TILE_SIZE * c;
				const uint32 messageSize = UPDATE_MESSAGE_HEADER_SIZE + sizeof(float) * h * w;

				mConnection->Out.write((uint32)messageSize);
				mConnection->Out.write((uint8)3);
				mConnection->Out.write((uint8)0);
				mConnection->Out.write(IMAGE_NAME);
				mConnection->Out.write(CHANNEL_NAMES[c]);
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