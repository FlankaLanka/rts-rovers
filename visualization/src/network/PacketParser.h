#pragma once

#include "common.h"

namespace terrafirma {

class PacketParser {
public:
    static bool parsePose(const char* data, size_t size, PosePacket& out);
    static bool parseLidarHeader(const char* data, size_t size, LidarPacketHeader& out);
    static bool parseTelemetry(const char* data, size_t size, VehicleTelem& out);
};

} // namespace terrafirma

