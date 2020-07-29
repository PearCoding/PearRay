#pragma once

#include "PR_Config.h"

namespace PR {
/* Protocol definition 
 * Incoming
 * [uint8] Type 
 * 
 * Outgoing
 * [uint8] Type
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

enum ProtocolType : uint8 {
	PT_PingRequest	  = 0x0,  // Dataless
	PT_PingResponse	  = 0x1,  // Dataless
	PT_InfoRequest	  = 0x2,  // TODO
	PT_InfoResponse	  = 0x3,  // TODO
	PT_StopRequest	  = 0x4,  // Dataless
	PT_StatusRequest  = 0x10, // Dataless
	PT_StatusResponse = 0x11,
	PT_ImageRequest	  = 0x12, // Dataless
	PT_ImageResponse  = 0x13,

	PT_MAX,
	PT_Invalid = 0xFF
};

struct PR_LIB_BASE ProtocolStatus {
	double Percentage;
	uint32 Iteration;
};

struct PR_LIB_BASE ProtocolImage {
	uint32 Width;
	uint32 Height;
	uint8 Type;
	//Float* Ptr!
};

class Serializer;
class PR_LIB_BASE Protocol {
public:
	static bool readHeader(Serializer& in, ProtocolType& type);
	static bool writeHeader(Serializer& in, ProtocolType type);

	static bool readStatus(Serializer& in, ProtocolStatus& status);
	static bool writeStatus(Serializer& in, const ProtocolStatus& status);

	// Should be header + data always -> But this way it allows buffer allocation etc
	static bool readImageHeader(Serializer& in, ProtocolImage& img);
	static bool readImageData(Serializer& in, const ProtocolImage& img, float* buffer, size_t bufferSize);
	static bool writeImage(Serializer& in, const ProtocolImage& img, const float* buffer, size_t bufferSize);
};
} // namespace PR