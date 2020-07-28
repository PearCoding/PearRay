#include <cmath>
#include <filesystem>
#include <iostream>

#include <cxxopts.hpp>

#include "network/Socket.h"
#include "serialization/NetworkSerializer.h"
#include <OpenImageIO/imageio.h>

namespace sf = std::filesystem;
using namespace PR;

static Socket* sConnection = nullptr;
void connect(const std::string& ip, uint16 port)
{
	if (sConnection && sConnection->isConnected())
		std::cout << "Already connected" << std::endl;
	else {
		if (!sConnection)
			sConnection = new Socket();
		std::cout << "Trying to connect to " << ip << ":" << port << std::endl;
		if (!sConnection->connect(port, ip)) {
			delete sConnection;
			sConnection = nullptr;
		}
	}
}

void disconnect()
{
	if (sConnection)
		delete sConnection;
	sConnection = nullptr;
}

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
			connect(args[1], 4217);
		} else {
			std::string ip	 = args[1].substr(0, it);
			std::string port = args[1].substr(it + 1);
			connect(ip, std::stoi(port));
		}
	} else {
		connect(args[1], std::stoi(args[2]));
	}
}

void handle_disconnect(const Arguments&)
{
	disconnect();
}

void handle_session(const Arguments&)
{
	if (sConnection && sConnection->isConnected())
		std::cout << "Connected to " << sConnection->ip() << ":" << sConnection->port() << std::endl;
	else
		std::cout << "Not connected" << std::endl;
}

bool ensureConnection()
{
	if (sConnection && sConnection->isConnected())
		return true;
	std::cout << "Not connected" << std::endl;
	return false;
}

void handle_stop(const Arguments&)
{
	if(!ensureConnection())
		return;

	NetworkSerializer serializer(sConnection, false);
	serializer.write(uint8(2));
}

void handle_status(const Arguments&)
{
	if(!ensureConnection())
		return;

	NetworkSerializer serializer(sConnection, false);
	serializer.write(uint8(0));
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
	{ "stop", "Request current session to stop", handle_stop },
	{ "status", "Request status information about current session", nullptr },
	{ "save", "Request image from current session and save it to disk", nullptr },
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
		connect(options.Ip, options.Port);

	bool stop = false;
	do {
		std::cout << ">> ";

		std::string line;
		std::getline(std::cin, line);
		trim(line);

		if (line == "exit")
			stop = true;
		else if (!line.empty())
			handle_line(line);
	} while (!stop);

	disconnect();

	return EXIT_SUCCESS;
}
