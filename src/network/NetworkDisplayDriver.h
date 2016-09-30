#pragma once

#include "renderer/DisplayDriver.h"

#include <asio.hpp>
#include <queue>
#include <list>
#include <mutex>

namespace PRN
{
	class PR_LIB_NETWORK NetworkDisplayDriver : public PR::IDisplayDriver
	{
		PR_CLASS_NON_COPYABLE(NetworkDisplayDriver);
	public:
		NetworkDisplayDriver(asio::io_service& io_service,
			const std::string& address, const PR::uint16 port);
		virtual ~NetworkDisplayDriver();

		void init(PR::Renderer* renderer) override;
		void deinit() override;

		void clear(PR::uint32 sx, PR::uint32 sy, PR::uint32 ex, PR::uint32 ey) override;

		void pushFragment(PR::uint32 x, PR::uint32 y, PR::uint32 layer, const PR::Spectrum& s) override;
		PR::Spectrum getFragment(PR::uint32 x, PR::uint32 y, PR::uint32 layer) const override;

		void handleIO(bool force);
	private:
		struct Packet
		{
			PR::uint32 X,Y,L;
			float R,G,B;
			
			template <typename Archive>
			void serialize(Archive& ar, const unsigned int version)
			{
				//ar & "PR"; ar & PR_VERSION
				ar & X; ar & Y;	ar & L;
				ar & R;	ar & G;	ar & B;
			}
		};

		class Connection
		{
		public:
			Connection(asio::io_service& io_service) :
			mSocket(io_service)
			{
			}

			inline asio::ip::tcp::socket& socket() { return mSocket; }

		private:
			asio::ip::tcp::socket mSocket;
		};

		void handle_accept(const asio::error_code& e, Connection* conn);

		std::queue<Packet> mPackets;
		std::vector<Packet> mBuffer;
		std::list<Connection*> mConnections;

		float* mData;
		PR::Renderer* mRenderer;

		asio::ip::tcp::endpoint mEndpoint;
		asio::ip::tcp::acceptor* mAcceptor;
		std::mutex mMutex;
	};
}