#include "Test.h"
#include "network/Socket.h"

#include <thread>

using namespace PR;

static bool ServerGood = false;
static bool ClientGood = false;

constexpr uint16_t PORT	 = 4242;
static const char DATA[] = "HELLO";
static void server()
{
	Socket socket;
	if (!socket.bind(PORT))
		return;
	if (!socket.listen(1))
		return;

	Socket client = socket.accept();
	if (!client.send(DATA, strlen(DATA) + 1))
		return;
	ServerGood = true;
}
static void client()
{
	Socket socket;
	if (!socket.connect(PORT, "127.0.0.1"))
		return;

	char buffer[64];
	if (!socket.receive(buffer, strlen(DATA) + 1))
		return;

	ClientGood = strcmp(buffer, DATA) == 0;
}

PR_BEGIN_TESTCASE(Network)
PR_TEST("Client")
{
	Socket socket;
	PR_CHECK_TRUE(socket.isValid());
	PR_CHECK_FALSE(socket.isConnection());
	bool result = socket.connect(PORT, "localhost");
	PR_CHECK_FALSE(result);
	PR_CHECK_FALSE(socket.isListening());
}
PR_TEST("Server")
{
	Socket socket;
	PR_CHECK_TRUE(socket.isValid());
	PR_CHECK_FALSE(socket.isConnection());
	bool result = socket.bind(PORT);
	PR_CHECK_TRUE(result);
	result = socket.listen(0);
	PR_CHECK_TRUE(result);
	PR_CHECK_TRUE(socket.isListening());
}
PR_TEST("Client-Server")
{
	std::thread t1(server);
	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	std::thread t2(client);

	t2.join();
	t1.join();

	PR_CHECK_TRUE(ServerGood);
	PR_CHECK_TRUE(ClientGood);
}
PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(Network);
PRT_END_MAIN