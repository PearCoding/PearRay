#include "NetworkObserver.h"
#include "Logger.h"
#include "ProgramSettings.h"
#include "buffer/FrameBufferSystem.h"
#include "network/Socket.h"
#include "renderer/RenderContext.h"
#include "serialization/BufferedNetworkSerializer.h"

#include <thread>

namespace PR {
enum IncomingType {
	IT_STATUSREQUEST = 0,
	IT_IMAGEREQUEST	 = 1,
	IT_STOPREQUEST	 = 2
};
enum OutgoingType {
	OT_STATUSRESPONSE = 0,
	OT_IMAGERESPONSE  = 1
};
/* Protocol definition 
 * Incoming
 * [uint8] Type { 0-> StatusRequest, 1-> ImageRequest, 2-> StopRequest }
 * 
 * Outgoing
 * [uint8] Type { 0-> StatusResponse, 1-> ImageResponse }
 * 
 * <Status>
 * [double] Percentage
 * [uint32] CurrentIteration
 * ... TODO ...
 * 
 * <Image>
 * [uint32] Width
 * [uint32] Height
 * [uint32] Format {0-> CIE XYZ, 1-> RGB} (Always triplet)
 * [float*Width*Height*3] Data
 */
void handle_status(NetworkObserver* observer, Serializer& out);
void handle_image(NetworkObserver* observer, Serializer& out);
bool handle_protocol(NetworkObserver* observer, BufferedNetworkSerializer& in, BufferedNetworkSerializer& out)
{
	uint8 in_type;
	in | in_type;
	PR_LOG(L_INFO) << "Incoming protocol type " << (int)in_type << std::endl;
	if (!in.isValid()) // Probably closed
		return false;

	switch (in_type) {
	case IT_STATUSREQUEST:
		handle_status(observer, out);
		break;
	case IT_IMAGEREQUEST:
		handle_image(observer, out);
		break;
	case IT_STOPREQUEST:
		observer->context()->stop();
		return false; // We can stop the client after this request
	default:
		PR_LOG(L_ERROR) << "Invalid incoming protocol type " << (int)in_type << std::endl;
		return false;
	}

	out.flush();
	return out.isValid();
}

void handle_status(NetworkObserver* observer, Serializer& out)
{
	RenderStatus status		= observer->context()->status();
	uint32 currentIteration = observer->currentIteration();

	out.write((uint8)OT_STATUSRESPONSE);
	out.write(status.percentage());
	out.write(currentIteration);
}

void handle_image(NetworkObserver* observer, Serializer& out)
{
	auto context = observer->context();
	auto channel = context->output()->data().getInternalChannel_Spectral();

	PR_ASSERT(channel->channels() == 3, "Expect spectral channel to have 3 channels"); // TODO: This may change in the future

	out.write((uint8)OT_IMAGERESPONSE);
	out.write((uint32)channel->width());
	out.write((uint32)channel->height());
	out.write((uint32)0);

	size_t size = channel->size().area() * 3 * sizeof(float);
	out.writeRaw(reinterpret_cast<const uint8*>(channel->ptr()), size);
}

// Server - Client
constexpr float TimeOut = 0.5f; //500ms
class NetworkServer;
class NetworkClient {
public:
	NetworkClient(Socket&& socket, NetworkServer* server)
		: mSocket(std::move(socket))
		, mServer(server)
	{
	}

	~NetworkClient() = default;

	void start();
	void stop()
	{
		if (mRunning) {
			mRunning = false;
			mThread.join();
		}
	}

	const Socket& socket() const { return mSocket; }

private:
	Socket mSocket;
	std::atomic<bool> mRunning;
	std::thread mThread;
	NetworkServer* mServer;
};

class NetworkServer {
public:
	NetworkServer(NetworkObserver* observer, int16 port)
		: mObserver(observer)
		, mPort(port)
	{
	}

	~NetworkServer() = default;

	void start()
	{
		mRunning = true;

		auto thread_func = [this]() {
			mSocket.bind(mPort);
			mSocket.listen();
			PR_LOG(L_INFO) << "Started server on " << mSocket.ip() << ":" << mSocket.port() << std::endl;

			while (mRunning) {
				if (mSocket.hasIncomingConnection(TimeOut)) {
					auto client = mSocket.accept();
					if (client.isValid() && client.isConnection())
						addClient(std::move(client));
				}
			}
		};

		mThread = std::thread(thread_func);
	}

	void stop()
	{
		mRunning = false;

		mMutex.lock();
		while (!mClients.empty()) {
			auto client = std::move(mClients.back());
			mClients.pop_back();
			mMutex.unlock();
			client->stop();
			mMutex.lock();
		}
		mMutex.unlock();

		mThread.join();
	}

	void addClient(Socket&& socket)
	{
		PR_LOG(L_INFO) << "Connected to " << socket.ip() << ":" << socket.port() << std::endl;
		auto client = std::make_unique<NetworkClient>(std::move(socket), this);
		client->start();

		mMutex.lock();
		mClients.push_back(std::move(client));
		mMutex.unlock();
	}

	void removeClient(NetworkClient* client)
	{
		PR_LOG(L_INFO) << "Disconnected from " << client->socket().ip() << ":" << client->socket().port() << std::endl;
		mMutex.lock();
		for (size_t i = 0; i < mClients.size(); ++i) {
			if (mClients[i].get() == client) {
				mClients.erase(mClients.begin() + i);
				break;
			}
		}
		mMutex.unlock();
	}

	inline NetworkObserver* observer() const { return mObserver; }

private:
	NetworkObserver* mObserver;
	int16 mPort;
	Socket mSocket;
	std::atomic<bool> mRunning;
	std::thread mThread;

	std::mutex mMutex;
	std::vector<std::unique_ptr<NetworkClient>> mClients;
};

// Main client based thread
void NetworkClient::start()
{
	mRunning = true;

	auto thread_func = [this]() {
		BufferedNetworkSerializer in(&mSocket, true);
		BufferedNetworkSerializer out(&mSocket, true);
		while (mRunning) {
			if (mSocket.hasData(TimeOut)) {
				if (!handle_protocol(mServer->observer(), in, out))
					break;
			}
			std::this_thread::yield();
		}

		mServer->removeClient(this);
	};

	mThread = std::thread(thread_func);
}

// Observer
NetworkObserver::NetworkObserver()
	: mRenderContext(nullptr)
	, mIterationCount(0)
{
}

NetworkObserver::~NetworkObserver()
{
	if (mServer)
		mServer->stop();
}

void NetworkObserver::begin(RenderContext* renderContext, const ProgramSettings& settings)
{
	PR_ASSERT(renderContext, "Invalid render context");

	if (mServer)
		mServer->stop();

	mServer			= std::make_unique<NetworkServer>(this, settings.ListenNetwork);
	mRenderContext	= renderContext;
	mIterationCount = 0;

	mServer->start();
}

void NetworkObserver::end()
{
	if (mServer)
		mServer->stop();

	mServer.reset();
}

void NetworkObserver::update(const UpdateInfo& info)
{
	mIterationCount = info.CurrentIteration;
}

void NetworkObserver::onIteration(const UpdateInfo& info)
{
	mIterationCount = info.CurrentIteration;
}

} // namespace PR