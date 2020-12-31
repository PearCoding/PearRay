#include "Protocol.h"
#include "serialization/Serializer.h"

namespace PR {
bool Protocol::readHeader(Serializer& in, ProtocolType& type)
{
	uint8 type_field;
	in.read(type_field);
	if ((ProtocolType)type_field < ProtocolType::MAX) {
		type = (ProtocolType)type_field;
		return in.isValid();
	} else {
		type = ProtocolType::Invalid;
		return false;
	}
}

bool Protocol::writeHeader(Serializer& in, ProtocolType type)
{
	if (type >= ProtocolType::MAX)
		return false;

	uint8 type_field = (uint8)type;
	in.write(type_field);
	return in.isValid();
}

bool Protocol::readStatus(Serializer& in, ProtocolStatus& status)
{
	in.read(status.Percentage);
	in.read(status.Iteration);
	return in.isValid();
}

bool Protocol::writeStatus(Serializer& in, const ProtocolStatus& status)
{
	in.write(status.Percentage);
	in.write(status.Iteration);
	return in.isValid();
}

bool Protocol::readImageHeader(Serializer& in, ProtocolImage& img)
{
	in.read(img.Width);
	in.read(img.Height);
	in.read(img.Type);
	return in.isValid();
}

bool Protocol::readImageData(Serializer& in, const ProtocolImage& img, float* buffer, size_t bufferSize)
{
	size_t requiredSize = img.Width * img.Height * 3;
	if (bufferSize < requiredSize)
		return false;

	in.readRaw(reinterpret_cast<uint8*>(buffer), requiredSize * sizeof(float));
	return in.isValid();
}

bool Protocol::writeImage(Serializer& in, const ProtocolImage& img, const float* buffer, size_t bufferSize)
{
	in.write(img.Width);
	in.write(img.Height);
	in.write(img.Type);

	size_t requiredSize = img.Width * img.Height * 3;
	if (bufferSize < requiredSize)
		return false;

	in.writeRaw(reinterpret_cast<const uint8*>(buffer), requiredSize * sizeof(float));
	return in.isValid();
}
} // namespace PR