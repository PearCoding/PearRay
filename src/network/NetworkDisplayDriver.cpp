#include "NetworkDisplayDriver.h"
#include "renderer/Renderer.h"

#include "spectral/RGBConverter.h"

#include <boost/bind.hpp>

#include <sstream>

namespace PRN
{
	using namespace PR;

	NetworkDisplayDriver::NetworkDisplayDriver(asio::io_service& io_service,
		 const std::string& address, const PR::uint16 port) :
		mData(nullptr), mRenderer(nullptr),
		mEndpoint(), mAcceptor(nullptr)
	{
		std::string port_s;
		std::stringstream stream;
		stream << port;
		stream >> port_s;

		asio::ip::tcp::resolver resolver(io_service);
  		asio::ip::tcp::resolver::query query(address, port_s);
		mEndpoint = *resolver.resolve(query);

		mAcceptor = new asio::ip::tcp::acceptor(io_service, mEndpoint);

		Connection* newCon = new Connection(mAcceptor->get_io_service());
		mAcceptor->async_accept(newCon->socket(),
			boost::bind(&NetworkDisplayDriver::handle_accept, this, asio::placeholders::error, newCon));
	}

	NetworkDisplayDriver::~NetworkDisplayDriver()
	{
		deinit();

		for(auto con : mConnections)
			delete con;

		delete mAcceptor;
	}

	void NetworkDisplayDriver::init(Renderer* renderer)
	{
		PR_ASSERT(renderer);
		PR_ASSERT(!mData);

		mRenderer = renderer;
		mData = new float[renderer->renderWidth()*renderer->renderHeight()*Spectrum::SAMPLING_COUNT];
		
		std::memset(mData, 0,
			mRenderer->renderWidth()*mRenderer->renderHeight()*Spectrum::SAMPLING_COUNT * sizeof(float));
	}

	void NetworkDisplayDriver::deinit()
	{
		if (mData)
		{
			delete[] mData;
			mData = nullptr;
		}
	}

	void NetworkDisplayDriver::clear(uint32 sx, uint32 sy, uint32 ex, uint32 ey)
	{
		if(!mData)
			return;

		if(sx < mRenderer->cropPixelOffsetX() && sy < mRenderer->cropPixelOffsetY() &&
			(ex == 0 || ex >= (mRenderer->cropPixelOffsetX() + mRenderer->renderWidth())) &&
			(ey == 0 || ey >= (mRenderer->cropPixelOffsetY() + mRenderer->renderHeight())))
		{// Full clear
			std::memset(mData, 0,
				mRenderer->renderWidth()*mRenderer->renderHeight()*Spectrum::SAMPLING_COUNT * sizeof(float));
		}
		else
		{
			PR_ASSERT(sx < ex);
			PR_ASSERT(sy < ey);
			
			sx = PM::pm_MaxT(sx, mRenderer->cropPixelOffsetX()) - mRenderer->cropPixelOffsetX();
			sy = PM::pm_MaxT(sy, mRenderer->cropPixelOffsetY()) - mRenderer->cropPixelOffsetY();

			ex = PM::pm_MinT(ex, mRenderer->cropPixelOffsetX() + mRenderer->renderWidth()) - mRenderer->cropPixelOffsetX();
			ey = PM::pm_MinT(ey, mRenderer->cropPixelOffsetY() + mRenderer->renderHeight()) - mRenderer->cropPixelOffsetY();

			for(uint32 y = sy; y < ey; ++y)// Line by line
			{
				std::memset(&mData[y*Spectrum::SAMPLING_COUNT + sx], 0,
					(ex - sx)*Spectrum::SAMPLING_COUNT * sizeof(float));
			}
		}
		// SEND PACKET
	}

	// TODO: No layer support
	void NetworkDisplayDriver::pushFragment(uint32 x, uint32 y, uint32 layer, const Spectrum& s)
	{
		PR_ASSERT(mData && mRenderer);

		PR_ASSERT(y < mRenderer->height());
		PR_ASSERT(y >= mRenderer->cropPixelOffsetY());
		PR_ASSERT(y < mRenderer->cropPixelOffsetY() + mRenderer->renderHeight());

		PR_ASSERT(x < mRenderer->width());
		PR_ASSERT(x >= mRenderer->cropPixelOffsetX());
		PR_ASSERT(x < mRenderer->cropPixelOffsetX() + mRenderer->renderWidth());

		const uint32 index = (y - mRenderer->cropPixelOffsetY())*mRenderer->renderWidth()*Spectrum::SAMPLING_COUNT 
			+ (x - mRenderer->cropPixelOffsetX())*Spectrum::SAMPLING_COUNT;

		for (uint32 i = 0; i < Spectrum::SAMPLING_COUNT; ++i)
			mData[index + i] = s.value(i);

		float r, g, b;
		PR::RGBConverter::convert(getFragment(x,y,0), r,g,b);

		mMutex.lock();
		mPackets.push(Packet({x,y,0,r,g,b}));
		mMutex.unlock();
	}

	Spectrum NetworkDisplayDriver::getFragment(uint32 x, uint32 y, uint32 layer) const
	{
		PR_ASSERT(mData && mRenderer);
		
		PR_ASSERT(y < mRenderer->height());
		PR_ASSERT(y >= mRenderer->cropPixelOffsetY());
		PR_ASSERT(y < mRenderer->cropPixelOffsetY() + mRenderer->renderHeight());

		PR_ASSERT(x < mRenderer->width());
		PR_ASSERT(x >= mRenderer->cropPixelOffsetX());
		PR_ASSERT(x < mRenderer->cropPixelOffsetX() + mRenderer->renderWidth());

		const uint32 index = (y - mRenderer->cropPixelOffsetY())*mRenderer->renderWidth()*Spectrum::SAMPLING_COUNT 
			+ (x - mRenderer->cropPixelOffsetX())*Spectrum::SAMPLING_COUNT;

		return Spectrum(&mData[index]);
	}

	constexpr size_t MIN_PACKAGE_COUNT = 1024;
	void NetworkDisplayDriver::handleIO(bool force)
	{
		if(mConnections.empty())
			return;
			
		mMutex.lock();
		if(mPackets.empty() || (!force && mPackets.size() < MIN_PACKAGE_COUNT))
		{
			mMutex.unlock();
			return;
		}
		
		while(force ? mPackets.empty() : (mPackets.size() > MIN_PACKAGE_COUNT))
		{
			const size_t packageCount = std::min(mPackets.size(), MIN_PACKAGE_COUNT);
			std::stringstream stream;
			stream << "PR " << PR_VERSION_STRING << std::endl
				<< packageCount << std::endl;

			for(size_t i = 0; i < packageCount && !mPackets.empty(); ++i)
			{
				Packet p = mPackets.front();
				mPackets.pop();
				
				stream << p.X << " " << p.Y << " " << p.L << " | " 
					<< p.R << " " << p.G << " " << p.B << std::endl;
			}

			for(auto con : mConnections)
				con->socket().send(asio::buffer(stream.str()));
		}
		mMutex.unlock();
	}

	void NetworkDisplayDriver::handle_accept(const asio::error_code& e, Connection* conn)
	{
		if(!e)
			mConnections.push_back(conn);
		else
			delete conn;
		
		Connection* newCon = new Connection(mAcceptor->get_io_service());
		mAcceptor->async_accept(newCon->socket(),
			boost::bind(&NetworkDisplayDriver::handle_accept, this, asio::placeholders::error, newCon));
	}
}