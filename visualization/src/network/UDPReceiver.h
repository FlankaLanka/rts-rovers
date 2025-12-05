#pragma once

#include "common.h"
#include "network/PacketParser.h"
#include <array>
#include <map>

namespace terrafirma {

class DataManager;

class UDPReceiver {
public:
    UDPReceiver(DataManager* dataManager);
    ~UDPReceiver();

    bool init();
    void update();
    void shutdown();
    
    void sendCommand(int roverId, uint8_t buttonStates);

private:
    void receivePackets();
    bool createSocket(int& sock, int port);
    void setNonBlocking(int sock);

    DataManager* m_dataManager;
    PacketParser m_parser;

    // Sockets for receiving (per rover)
    std::array<int, NUM_ROVERS> m_poseSockets;
    std::array<int, NUM_ROVERS> m_lidarSockets;
    std::array<int, NUM_ROVERS> m_telemSockets;
    
    // Socket for sending commands
    int m_cmdSocket = -1;

    // LiDAR chunk reassembly (per rover)
    struct LidarScanBuilder {
        double timestamp = 0.0;
        uint32_t totalChunks = 0;
        uint32_t receivedChunks = 0;
        std::vector<LidarPoint> points;
        std::vector<bool> chunkReceived;
    };
    std::array<std::map<double, LidarScanBuilder>, NUM_ROVERS> m_lidarBuilders;

    bool m_initialized = false;
};

} // namespace terrafirma

