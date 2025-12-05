#include "network/PacketParser.h"
#include <cstring>

namespace terrafirma {

bool PacketParser::parsePose(const char* data, size_t size, PosePacket& out) {
    if (size < sizeof(PosePacket)) return false;
    std::memcpy(&out, data, sizeof(PosePacket));
    return true;
}

bool PacketParser::parseLidarHeader(const char* data, size_t size, LidarPacketHeader& out) {
    if (size < sizeof(LidarPacketHeader)) return false;
    std::memcpy(&out, data, sizeof(LidarPacketHeader));
    return true;
}

bool PacketParser::parseTelemetry(const char* data, size_t size, VehicleTelem& out) {
    if (size < sizeof(VehicleTelem)) return false;
    std::memcpy(&out, data, sizeof(VehicleTelem));
    return true;
}

} // namespace terrafirma

