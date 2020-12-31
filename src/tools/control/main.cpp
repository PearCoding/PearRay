#include <cmath>
#include <filesystem>
#include <iostream>

#include <cxxopts.hpp>

#include "network/Protocol.h"
#include "network/Socket.h"
#include "serialization/BufferedNetworkSerializer.h"
#include <OpenImageIO/imageio.h>

namespace sf = std::filesystem;
using namespace PR;

struct Connection {
	::Socket* Socket = nullptr;
	BufferedNetworkSerializer In;
	BufferedNetworkSerializer Out;
	std::vector<float> ImageBuffer;

	~Connection() { disconnect(); }
	inline void connect(const std::string& ip, uint16 port)
	{
		if (Socket && Socket->isOpen())
			std::cout << "Already connected" << std::endl;
		else {
			if (Socket)
				delete Socket;
			Socket = new ::Socket();
			std::cout << "Trying to connect to " << ip << ":" << port << std::endl;
			if (!Socket->connect(port, ip)) {
				delete Socket;
				Socket = nullptr;
			} else {
				In.setSocket(Socket, true);
				Out.setSocket(Socket, false);
			}
		}
	}

	inline void disconnect()
	{
		if (Socket)
			delete Socket;
		Socket = nullptr;
	}

	inline bool isOpen() { return Socket && Socket->isOpen(); }
};

static bool sQuit = false;
static Connection sConnection;

class ProgramSettings {
public:
	std::string Ip;
	uint16 Port;

	bool IsVerbose;

	bool parse(int argc, char** argv);
};

bool ProgramSettings::parse(int argc, char** argv)
{
	try {
		cxxopts::Options options("primg2coeff", "Convert image with given coefficent file into a preprocessed image");

		// clang-format off
		options.add_options()
			("h,help", "Produce this help message")
			("v,verbose", "Print detailed information")
			("ip", "Ip", cxxopts::value<std::string>())
			("port", "Port", cxxopts::value<uint16>()->default_value("4217"))
		;
		// clang-format on
		options.parse_positional({ "ip" });

		auto vm = options.parse(argc, argv);

		// Handle help
		if (vm.count("help")) {
			std::cout << "See Wiki for more information:\n  https://github.com/PearCoding/PearRay/wiki\n"
					  << std::endl;
			std::cout << options.help() << std::endl;
			exit(0);
		}

		// Input file
		if (vm.count("ip"))
			Ip = vm["ip"].as<std::string>();
		Port = vm["port"].as<uint16>();

		IsVerbose = (vm.count("verbose") != 0);
	} catch (const cxxopts::OptionException& e) {
		std::cout << "Error while parsing commandline: " << e.what() << std::endl;
		return false;
	}

	return true;
}

bool write_output(const std::string& filename, const std::vector<float>& data, int width, int height)
{
	auto out = OIIO::ImageOutput::create(filename);
	if (!out) {
		std::cerr << "Error: Could not create output file" << std::endl;
		return false;
	}

	OIIO::ImageSpec spec(width, height, 3, OIIO::TypeDesc::FLOAT);
	spec.attribute("oiio:ColorSpace", "CIE XYZ");
	spec.attribute("Software", "PearRay control tool");

	out->open(filename, spec);
	out->write_image(OIIO::TypeDesc::FLOAT, &data[0]);
	out->close();

	return true;
}

static inline void ltrim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
				return !std::isspace(ch);
			}));
}

static inline void rtrim(std::string& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
				return !std::isspace(ch);
			}).base(),
			s.end());
}
static inline void trim(std::string& s)
{
	ltrim(s);
	rtrim(s);
}

using Arguments = std::vector<std::string>;
Arguments split_args(const std::string& cmd)
{
	Arguments args;
	std::string current_arg = "";

	for (auto it = cmd.begin(); it != cmd.end();) {
		if (std::isspace(*it)) {
			if (!current_arg.empty()) {
				args.push_back(current_arg);
				current_arg = "";
			}
			++it;
		} else if (*it == '\\') {
			++it;
			if (it != cmd.end()) {
				current_arg += *it;
				++it;
			}
		} else if (*it == '"') {
			++it;
			while (it != cmd.end()) {
				if (*it == '\\') {
					++it;
					if (it != cmd.end()) {
						current_arg += *it;
						++it;
					}
				} else if (*it == '"') {
					++it;
					break;
				} else {
					current_arg += *it;
					++it;
				}
			}
		} else {
			current_arg += *it;
			++it;
		}
	}

	if (!current_arg.empty())
		args.push_back(current_arg);
	return args;
}

void handle_connect(const Arguments& args)
{
	if (args.size() < 2) {
		std::cout << "Usage: IP [Port]" << std::endl;
		return;
	}

	if (args.size() == 2) {
		auto it = args[1].find_last_of(':');
		if (it == std::string::npos) {
			sConnection.connect(args[1], 4217);
		} else {
			std::string ip	 = args[1].substr(0, it);
			std::string port = args[1].substr(it + 1);
			sConnection.connect(ip, std::stoi(port));
		}
	} else {
		sConnection.connect(args[1], std::stoi(args[2]));
	}
}

void handle_disconnect(const Arguments&)
{
	sConnection.disconnect();
}

void handle_session(const Arguments&)
{
	if (sConnection.isOpen())
		std::cout << "Connected to " << sConnection.Socket->ip() << ":" << sConnection.Socket->port() << std::endl;
	else
		std::cout << "Not connected" << std::endl;
}

bool ensureConnection()
{
	if (sConnection.isOpen())
		return true;
	std::cout << "Not connected" << std::endl;
	return false;
}

void notifyDisonnection()
{
	if (!sConnection.isOpen())
		std::cout << "Disconnected" << std::endl;
}

void handle_stop(const Arguments&)
{
	if (!ensureConnection())
		return;

	Protocol::writeHeader(sConnection.Out, ProtocolType::StopRequest);
	sConnection.Out.flush();
}

void handle_ping(const Arguments&)
{
	if (!ensureConnection())
		return;

	auto start = std::chrono::high_resolution_clock::now();
	Protocol::writeHeader(sConnection.Out, ProtocolType::PingRequest);
	sConnection.Out.flush();

	ProtocolType type;
	if (!Protocol::readHeader(sConnection.In, type)) {
		std::cout << "Could not get protocol header" << std::endl;
	} else {
		auto end = std::chrono::high_resolution_clock::now();
		if (type == ProtocolType::PingResponse)
			std::cout << "Successful [" << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms]" << std::endl;
		else
			std::cout << "Got unexpected response " << (uint8)type << std::endl;
	}
	notifyDisonnection();
}

void handle_status(const Arguments&)
{
	if (!ensureConnection())
		return;

	Protocol::writeHeader(sConnection.Out, ProtocolType::StatusRequest);
	sConnection.Out.flush();

	ProtocolType type;
	if (!Protocol::readHeader(sConnection.In, type)) {
		std::cout << "Could not get protocol header" << std::endl;
	} else {
		if (type == ProtocolType::StatusResponse) {
			ProtocolStatus status;
			Protocol::readStatus(sConnection.In, status);
			std::cout << status.Percentage << "% " << status.Iteration << std::endl;
		} else
			std::cout << "Got unexpected response " << (uint8)type << std::endl;
	}
	notifyDisonnection();
}

void handle_save(const Arguments& args)
{
	if (!ensureConnection())
		return;

	Protocol::writeHeader(sConnection.Out, ProtocolType::ImageRequest);
	sConnection.Out.flush();

	ProtocolType type;
	if (!Protocol::readHeader(sConnection.In, type)) {
		std::cout << "Could not get protocol header" << std::endl;
	} else {
		if (type == ProtocolType::ImageResponse) {
			ProtocolImage image;
			if (!Protocol::readImageHeader(sConnection.In, image)) {
				std::cout << "Could not get image header" << std::endl;
			} else {
				size_t requestedSize = image.Width * image.Height * 3;
				if (sConnection.ImageBuffer.size() < requestedSize)
					sConnection.ImageBuffer.resize(requestedSize);

				if (!Protocol::readImageData(sConnection.In, image, sConnection.ImageBuffer.data(), sConnection.ImageBuffer.size())) {
					std::cout << "Could not get image data" << std::endl;
				} else {
					std::string filename = "image.exr";
					if (args.size() > 1)
						filename = args[1];

					if (write_output(filename, sConnection.ImageBuffer, image.Width, image.Height))
						std::cout << "Save image to file " << filename << std::endl;
					else
						std::cout << "Could not save image to file " << filename << std::endl;
				}
			}
		} else
			std::cout << "Got unexpected response " << (uint8)type << std::endl;
	}
	notifyDisonnection();
}

void handle_quit(const Arguments&)
{
	sQuit = true;
}

void handle_help(const Arguments&);
using Command = void (*)(const Arguments&);
struct {
	const char* Name;
	const char* Description;
	Command Cmd;
} cmds[] = {
	{ "help", "Print information about available commands", handle_help },
	{ "connect", "Connect to a running PearRay session", handle_connect },
	{ "disconnect", "Disconnect from a running PearRay session", handle_disconnect },
	{ "session", "Display information about current session", handle_session },
	{ "ping", "Ping current session to stop", handle_ping },
	{ "stop", "Request current session to stop", handle_stop },
	{ "status", "Request status information about current session", handle_status },
	{ "save", "Request image from current session and save it to disk", handle_save },
	{ "quit", "Exit program", handle_quit },
	{ "exit", "Exit program", handle_quit },
	{ nullptr, nullptr, nullptr }
};

void handle_help(const Arguments&)
{
	std::cout << "Available commands:" << std::endl;
	auto whitespacetill = [](const char* cmd, size_t s) {
		for (size_t i = 0; i < s - strlen(cmd); ++i)
			std::cout << " ";
	};

	size_t wp = 0;
	for (size_t i = 0; cmds[i].Name; ++i) {
		wp = std::max(wp, strlen(cmds[i].Name));
	}
	wp += 5;

	for (size_t i = 0; cmds[i].Name; ++i) {
		std::cout << "  " << cmds[i].Name;
		whitespacetill(cmds[i].Name, wp);
		std::cout << cmds[i].Description << std::endl;
	}
}

void handle_line(const std::string& line)
{
	std::vector<std::string> args = split_args(line);
	if (args.empty())
		return;
	/*for (auto arg : args)
		std::cout << arg << std::endl;*/

	Command cmd = nullptr;
	for (size_t i = 0; cmds[i].Name; ++i) {
		if (cmds[i].Name == args[0]) {
			cmd = cmds[i].Cmd;
			break;
		}
	}

	if (cmd)
		cmd(args);
	else
		std::cout << "Unknown command: " << args[0] << std::endl;
}

int main(int argc, char** argv)
{
	ProgramSettings options;
	if (!options.parse(argc, argv))
		return EXIT_FAILURE;

	if (options.IsVerbose) {
		std::cout << "Arguments> " << std::endl;
		std::cout << "  Ip:   " << options.Ip << std::endl;
		std::cout << "  Port:  " << options.Port << std::endl;
	}

	if (!options.Ip.empty())
		sConnection.connect(options.Ip, options.Port);

	sQuit = false;
	do {
		std::cout << ">> ";

		std::string line;
		std::getline(std::cin, line);
		trim(line);

		handle_line(line);
	} while (!sQuit);

	sConnection.disconnect();

	return EXIT_SUCCESS;
}
