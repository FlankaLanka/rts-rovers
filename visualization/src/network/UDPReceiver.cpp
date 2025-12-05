#include "network/UDPReceiver.h"
#include "data/DataManager.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

namespace terrafirma {

UDPReceiver::UDPReceiver(DataManager* dataManager)
    : m_dataManager(dataManager) {
    m_poseSockets.fill(-1);
    m_lidarSockets.fill(-1);
    m_telemSockets.fill(-1);
}

UDPReceiver::~UDPReceiver() {
    shutdown();
}

bool UDPReceiver::init() {
    // Create command sending socket
    m_cmdSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_cmdSocket < 0) {
        std::cerr << "Failed to create command socket\n";
        return false;
    }

    // Create receiving sockets for each rover
    for (int i = 0; i < NUM_ROVERS; i++) {
        int roverId = i + 1;
        
        if (!createSocket(m_poseSockets[i], POSE_PORT_BASE + roverId)) {
            std::cerr << "Failed to create pose socket for rover " << roverId << "\n";
            return false;
        }
        
        if (!createSocket(m_lidarSockets[i], LIDAR_PORT_BASE + roverId)) {
            std::cerr << "Failed to create lidar socket for rover " << roverId << "\n";
            return false;
        }
        
        if (!createSocket(m_telemSockets[i], TELEM_PORT_BASE + roverId)) {
            std::cerr << "Failed to create telemetry socket for rover " << roverId << "\n";
            return false;
        }

        setNonBlocking(m_poseSockets[i]);
        setNonBlocking(m_lidarSockets[i]);
        setNonBlocking(m_telemSockets[i]);
    }

    m_initialized = true;
    std::cout << "Network receiver initialized\n";
    return true;
}

bool UDPReceiver::createSocket(int& sock, int port) {
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return false;
    }

    // Allow address reuse
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#ifdef SO_REUSEPORT
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "Failed to bind to port " << port << ": " << strerror(errno) << "\n";
        close(sock);
        sock = -1;
        return false;
    }

    return true;
}

void UDPReceiver::setNonBlocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

void UDPReceiver::update() {
    if (!m_initialized) return;
    receivePackets();
}

void UDPReceiver::receivePackets() {
    char buffer[2048];

    for (int i = 0; i < NUM_ROVERS; i++) {
        int roverId = i + 1;

        // Receive pose packets
        while (true) {
            ssize_t n = recv(m_poseSockets[i], buffer, sizeof(buffer), 0);
            if (n <= 0) break;
            
            if (n == sizeof(PosePacket)) {
                PosePacket pose;
                std::memcpy(&pose, buffer, sizeof(PosePacket));
                m_dataManager->updateRoverPose(roverId, pose);
            }
        }

        // Receive telemetry packets
        while (true) {
            ssize_t n = recv(m_telemSockets[i], buffer, sizeof(buffer), 0);
            if (n <= 0) break;
            
            if (n == sizeof(VehicleTelem)) {
                VehicleTelem telem;
                std::memcpy(&telem, buffer, sizeof(VehicleTelem));
                m_dataManager->updateRoverTelemetry(roverId, telem);
            }
        }

        // Receive LiDAR packets
        while (true) {
            ssize_t n = recv(m_lidarSockets[i], buffer, sizeof(buffer), 0);
            if (n <= 0) break;
            
            if (n >= static_cast<ssize_t>(sizeof(LidarPacketHeader))) {
                LidarPacketHeader header;
                std::memcpy(&header, buffer, sizeof(LidarPacketHeader));
                
                // Get or create scan builder for this timestamp
                auto& builders = m_lidarBuilders[i];
                auto& builder = builders[header.timestamp];
                
                if (builder.totalChunks == 0) {
                    builder.timestamp = header.timestamp;
                    builder.totalChunks = header.totalChunks;
                    builder.receivedChunks = 0;
                    builder.points.reserve(header.totalChunks * MAX_LIDAR_POINTS_PER_PACKET);
                    builder.chunkReceived.resize(header.totalChunks, false);
                }

                // Store points from this chunk
                if (header.chunkIndex < builder.totalChunks && !builder.chunkReceived[header.chunkIndex]) {
                    builder.chunkReceived[header.chunkIndex] = true;
                    builder.receivedChunks++;
                    
                    const LidarPoint* points = reinterpret_cast<const LidarPoint*>(buffer + sizeof(LidarPacketHeader));
                    for (uint32_t j = 0; j < header.pointsInThisChunk; j++) {
                        builder.points.push_back(points[j]);
                    }
                }

                // Check if scan is complete
                if (builder.receivedChunks >= builder.totalChunks) {
                    m_dataManager->addPointCloud(roverId, builder.points);
                    builders.erase(header.timestamp);
                }

                // Clean up old incomplete scans (older than 1 second)
                auto it = builders.begin();
                while (it != builders.end()) {
                    if (header.timestamp - it->first > 1.0) {
                        it = builders.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }
    }
}

void UDPReceiver::sendCommand(int roverId, uint8_t buttonStates) {
    if (m_cmdSocket < 0 || roverId < 1 || roverId > NUM_ROVERS) return;

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CMD_PORT_BASE + roverId);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    sendto(m_cmdSocket, &buttonStates, 1, 0,
           reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
}

void UDPReceiver::shutdown() {
    for (int i = 0; i < NUM_ROVERS; i++) {
        if (m_poseSockets[i] >= 0) close(m_poseSockets[i]);
        if (m_lidarSockets[i] >= 0) close(m_lidarSockets[i]);
        if (m_telemSockets[i] >= 0) close(m_telemSockets[i]);
    }
    if (m_cmdSocket >= 0) close(m_cmdSocket);
    
    m_poseSockets.fill(-1);
    m_lidarSockets.fill(-1);
    m_telemSockets.fill(-1);
    m_cmdSocket = -1;
    m_initialized = false;
}

} // namespace terrafirma

