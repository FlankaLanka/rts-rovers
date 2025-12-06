#include "render/RoverRenderer.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stack>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

namespace terrafirma {

// Shader sources
static const char* ROVER_VERT_SRC = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

static const char* ROVER_FRAG_SRC = R"(
#version 330 core
in vec3 Normal;
in vec3 FragPos;

uniform vec3 color;
uniform float alpha;
uniform bool selected;
uniform bool engineRunning;

out vec4 FragColor;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(Normal), lightDir), 0.0);
    float ambient = 0.3;
    
    vec3 result = color * (ambient + diff * 0.7);
    
    if (!engineRunning) {
        float gray = dot(result, vec3(0.299, 0.587, 0.114));
        result = mix(vec3(gray), result, 0.3) * 0.5;
        result += vec3(0.1, 0.0, 0.0);
    }
    
    if (selected) {
        result += vec3(0.2, 0.4, 0.5);
    }
    
    FragColor = vec4(result, alpha);
}
)";

static const std::vector<std::string> MODEL_PATHS = {
    "assets/models/rover.glb",
    "assets/models/rover.gltf",
    "assets/models/rover.obj"
};

// Target model size in world units (increased 3x from 5 to 15)
static const float TARGET_MODEL_SIZE = 15.0f;

RoverRenderer::RoverRenderer() {}

RoverRenderer::~RoverRenderer() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
}

bool RoverRenderer::init() {
    if (!m_shader.loadFromSource(ROVER_VERT_SRC, ROVER_FRAG_SRC)) {
        return false;
    }
    
    if (!loadModel()) {
        std::cout << "\n=== ROVER MODEL SETUP ===\n";
        std::cout << "No model found. Using fallback mesh.\n";
        std::cout << "To use a custom model, place one of these files:\n";
        std::cout << "  visualization/assets/models/rover.glb  (recommended)\n";
        std::cout << "  visualization/assets/models/rover.gltf\n";
        std::cout << "  visualization/assets/models/rover.obj\n";
        std::cout << "=========================\n\n";
        createFallbackMesh();
    }
    
    return true;
}

bool RoverRenderer::loadModel() {
    for (const auto& path : MODEL_PATHS) {
        std::ifstream file(path);
        if (!file.good()) continue;
        file.close();
        
        std::string ext = path.substr(path.find_last_of('.'));
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        bool success = false;
        if (ext == ".obj") {
            success = loadOBJ(path);
        } else if (ext == ".gltf" || ext == ".glb") {
            success = loadGLTF(path);
        }
        
        if (success) {
            std::cout << "Loaded rover model: " << path << std::endl;
            return true;
        }
    }
    return false;
}

bool RoverRenderer::loadOBJ(const std::string& path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
    if (!ret || attrib.vertices.empty()) return false;
    
    float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
    float maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;
    
    for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
        minX = std::min(minX, attrib.vertices[i]);
        maxX = std::max(maxX, attrib.vertices[i]);
        minY = std::min(minY, attrib.vertices[i + 1]);
        maxY = std::max(maxY, attrib.vertices[i + 1]);
        minZ = std::min(minZ, attrib.vertices[i + 2]);
        maxZ = std::max(maxZ, attrib.vertices[i + 2]);
    }
    
    float maxSize = std::max({maxX - minX, maxY - minY, maxZ - minZ});
    m_modelScale = TARGET_MODEL_SIZE / maxSize;
    
    float centerX = (minX + maxX) / 2.0f;
    float centerY = (minY + maxY) / 2.0f;
    float centerZ = (minZ + maxZ) / 2.0f;
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int indexOffset = 0;
    
    for (const auto& shape : shapes) {
        for (size_t f = 0; f < shape.mesh.indices.size(); f++) {
            tinyobj::index_t idx = shape.mesh.indices[f];
            
            vertices.push_back((attrib.vertices[3 * idx.vertex_index + 0] - centerX) * m_modelScale);
            vertices.push_back((attrib.vertices[3 * idx.vertex_index + 1] - centerY) * m_modelScale);
            vertices.push_back((attrib.vertices[3 * idx.vertex_index + 2] - centerZ) * m_modelScale);
            
            if (idx.normal_index >= 0) {
                vertices.push_back(attrib.normals[3 * idx.normal_index + 0]);
                vertices.push_back(attrib.normals[3 * idx.normal_index + 1]);
                vertices.push_back(attrib.normals[3 * idx.normal_index + 2]);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);
            }
            indices.push_back(indexOffset++);
        }
    }
    
    if (vertices.empty()) return false;
    
    m_indexCount = indices.size();
    
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    
    m_modelLoaded = true;
    std::cout << "  Format: OBJ, Triangles: " << m_indexCount / 3 << std::endl;
    return true;
}

// Helper function to get node's local transform matrix
glm::mat4 getNodeTransform(const tinygltf::Node& node) {
    glm::mat4 transform(1.0f);
    
    if (node.matrix.size() == 16) {
        // Use the matrix directly
        transform = glm::make_mat4(node.matrix.data());
    } else {
        // Build from TRS
        if (node.translation.size() == 3) {
            transform = glm::translate(transform, glm::vec3(
                static_cast<float>(node.translation[0]),
                static_cast<float>(node.translation[1]),
                static_cast<float>(node.translation[2])
            ));
        }
        if (node.rotation.size() == 4) {
            glm::quat q(
                static_cast<float>(node.rotation[3]),  // w
                static_cast<float>(node.rotation[0]),  // x
                static_cast<float>(node.rotation[1]),  // y
                static_cast<float>(node.rotation[2])   // z
            );
            transform *= glm::mat4_cast(q);
        }
        if (node.scale.size() == 3) {
            transform = glm::scale(transform, glm::vec3(
                static_cast<float>(node.scale[0]),
                static_cast<float>(node.scale[1]),
                static_cast<float>(node.scale[2])
            ));
        }
    }
    
    return transform;
}

bool RoverRenderer::loadGLTF(const std::string& path) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;
    
    std::string ext = path.substr(path.find_last_of('.'));
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    bool ret = (ext == ".glb") ? 
        loader.LoadBinaryFromFile(&model, &err, &warn, path) :
        loader.LoadASCIIFromFile(&model, &err, &warn, path);
    
    if (!warn.empty()) {
        std::cout << "GLTF Warning: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "GLTF Error: " << err << std::endl;
    }
    if (!ret) {
        std::cerr << "Failed to load GLTF file: " << path << std::endl;
        return false;
    }
    
    std::cout << "GLTF loaded: " << model.meshes.size() << " meshes, " << model.nodes.size() << " nodes" << std::endl;
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Build node transform hierarchy - traverse the scene graph
    std::vector<glm::mat4> nodeWorldTransforms(model.nodes.size(), glm::mat4(1.0f));
    
    // Process nodes to build world transforms
    // First, compute local transforms
    for (size_t i = 0; i < model.nodes.size(); i++) {
        nodeWorldTransforms[i] = getNodeTransform(model.nodes[i]);
    }
    
    // Then, traverse from scene roots and accumulate transforms
    std::function<void(int, const glm::mat4&)> processNode = [&](int nodeIdx, const glm::mat4& parentTransform) {
        if (nodeIdx < 0 || nodeIdx >= static_cast<int>(model.nodes.size())) return;
        
        const tinygltf::Node& node = model.nodes[nodeIdx];
        glm::mat4 localTransform = getNodeTransform(node);
        glm::mat4 worldTransform = parentTransform * localTransform;
        nodeWorldTransforms[nodeIdx] = worldTransform;
        
        // Process children
        for (int childIdx : node.children) {
            processNode(childIdx, worldTransform);
        }
    };
    
    // Process all scenes (usually just one)
    for (const auto& scene : model.scenes) {
        for (int nodeIdx : scene.nodes) {
            processNode(nodeIdx, glm::mat4(1.0f));
        }
    }
    
    // If no scenes, process all root nodes
    if (model.scenes.empty()) {
        for (size_t i = 0; i < model.nodes.size(); i++) {
            // Check if this is a root node (not a child of any other node)
            bool isRoot = true;
            for (const auto& n : model.nodes) {
                for (int childIdx : n.children) {
                    if (childIdx == static_cast<int>(i)) {
                        isRoot = false;
                        break;
                    }
                }
                if (!isRoot) break;
            }
            if (isRoot) {
                processNode(static_cast<int>(i), glm::mat4(1.0f));
            }
        }
    }
    
    // Now process each node that has a mesh
    for (size_t nodeIdx = 0; nodeIdx < model.nodes.size(); nodeIdx++) {
        const tinygltf::Node& node = model.nodes[nodeIdx];
        if (node.mesh < 0) continue;
        
        const tinygltf::Mesh& mesh = model.meshes[node.mesh];
        glm::mat4 worldTransform = nodeWorldTransforms[nodeIdx];
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(worldTransform)));
        
        for (const auto& primitive : mesh.primitives) {
            // Support triangles and default mode
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES && primitive.mode != -1) {
                continue;
            }
            
            auto posIt = primitive.attributes.find("POSITION");
            if (posIt == primitive.attributes.end()) continue;
            
            const tinygltf::Accessor& posAccessor = model.accessors[posIt->second];
            const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
            const tinygltf::Buffer& posBuffer = model.buffers[posView.buffer];
            
            // Get byte stride for positions
            size_t posByteStride = posView.byteStride;
            if (posByteStride == 0) posByteStride = 3 * sizeof(float);
            
            // Check for normals
            const tinygltf::Accessor* normAccessor = nullptr;
            const tinygltf::BufferView* normView = nullptr;
            const tinygltf::Buffer* normBuffer = nullptr;
            size_t normByteStride = 0;
            
            auto normIt = primitive.attributes.find("NORMAL");
            if (normIt != primitive.attributes.end()) {
                normAccessor = &model.accessors[normIt->second];
                normView = &model.bufferViews[normAccessor->bufferView];
                normBuffer = &model.buffers[normView->buffer];
                normByteStride = normView->byteStride;
                if (normByteStride == 0) normByteStride = 3 * sizeof(float);
            }
            
            unsigned int baseVertex = static_cast<unsigned int>(vertices.size() / 6);
            
            // Read vertices with transform applied
            for (size_t i = 0; i < posAccessor.count; i++) {
                const unsigned char* posData = &posBuffer.data[posView.byteOffset + posAccessor.byteOffset + i * posByteStride];
                const float* pos = reinterpret_cast<const float*>(posData);
                
                // Apply world transform to position
                glm::vec4 worldPos = worldTransform * glm::vec4(pos[0], pos[1], pos[2], 1.0f);
                
                vertices.push_back(worldPos.x);
                vertices.push_back(worldPos.y);
                vertices.push_back(worldPos.z);
                
                if (normAccessor) {
                    const unsigned char* normData = &normBuffer->data[normView->byteOffset + normAccessor->byteOffset + i * normByteStride];
                    const float* norm = reinterpret_cast<const float*>(normData);
                    
                    // Apply normal matrix to normal
                    glm::vec3 worldNorm = glm::normalize(normalMatrix * glm::vec3(norm[0], norm[1], norm[2]));
                    
                    vertices.push_back(worldNorm.x);
                    vertices.push_back(worldNorm.y);
                    vertices.push_back(worldNorm.z);
                } else {
                    vertices.push_back(0.0f);
                    vertices.push_back(1.0f);
                    vertices.push_back(0.0f);
                }
            }
            
            // Read indices
            if (primitive.indices >= 0) {
                const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& indexView = model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer& indexBuffer = model.buffers[indexView.buffer];
                
                const unsigned char* indexData = &indexBuffer.data[indexView.byteOffset + indexAccessor.byteOffset];
                
                for (size_t i = 0; i < indexAccessor.count; i++) {
                    unsigned int index;
                    switch (indexAccessor.componentType) {
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            index = reinterpret_cast<const uint16_t*>(indexData)[i];
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                            index = reinterpret_cast<const uint32_t*>(indexData)[i];
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            index = indexData[i];
                            break;
                        default:
                            index = 0;
                    }
                    indices.push_back(baseVertex + index);
                }
            } else {
                // No indices - generate sequential
                for (size_t i = 0; i < posAccessor.count; i++) {
                    indices.push_back(baseVertex + static_cast<unsigned int>(i));
                }
            }
        }
    }
    
    if (vertices.empty()) {
        std::cerr << "No vertices loaded from GLTF" << std::endl;
        return false;
    }
    
    std::cout << "Total: " << vertices.size() / 6 << " vertices, " << indices.size() / 3 << " triangles" << std::endl;
    
    // Calculate bounding box
    float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
    float maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;
    for (size_t i = 0; i < vertices.size(); i += 6) {
        minX = std::min(minX, vertices[i]);
        maxX = std::max(maxX, vertices[i]);
        minY = std::min(minY, vertices[i + 1]);
        maxY = std::max(maxY, vertices[i + 1]);
        minZ = std::min(minZ, vertices[i + 2]);
        maxZ = std::max(maxZ, vertices[i + 2]);
    }
    
    std::cout << "Bounds: X[" << minX << ", " << maxX << "] Y[" << minY << ", " << maxY << "] Z[" << minZ << ", " << maxZ << "]" << std::endl;
    
    // Normalize: scale to target size and center
    float sizeX = maxX - minX;
    float sizeY = maxY - minY;
    float sizeZ = maxZ - minZ;
    float maxSize = std::max({sizeX, sizeY, sizeZ});
    m_modelScale = TARGET_MODEL_SIZE / maxSize;
    
    float centerX = (minX + maxX) / 2.0f;
    float centerY = minY;  // Put origin at bottom of model (ground level)
    float centerZ = (minZ + maxZ) / 2.0f;
    
    // Apply centering and scaling
    for (size_t i = 0; i < vertices.size(); i += 6) {
        vertices[i] = (vertices[i] - centerX) * m_modelScale;
        vertices[i + 1] = (vertices[i + 1] - centerY) * m_modelScale;
        vertices[i + 2] = (vertices[i + 2] - centerZ) * m_modelScale;
    }
    
    m_indexCount = indices.size();
    
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    
    m_modelLoaded = true;
    std::cout << "  Format: GLTF/GLB, Scale: " << m_modelScale << ", Final size: ~" << TARGET_MODEL_SIZE << " units" << std::endl;
    return true;
}

void RoverRenderer::createFallbackMesh() {
    float hw = 2.0f, hd = 1.5f, hh = 1.0f;
    
    std::vector<float> vertices = {
        -hw, -hh, -hd,  0, -1, 0,   hw, -hh, -hd,  0, -1, 0,
         hw, -hh,  hd,  0, -1, 0,  -hw, -hh,  hd,  0, -1, 0,
        -hw, hh, -hd,  0, 1, 0,    hw, hh, -hd,  0, 1, 0,
         hw, hh,  hd,  0, 1, 0,   -hw, hh,  hd,  0, 1, 0,
        -hw, -hh, hd,  0, 0, 1,    hw, -hh, hd,  0, 0, 1,
         hw,  hh, hd,  0, 0, 1,   -hw,  hh, hd,  0, 0, 1,
        -hw, -hh, -hd,  0, 0, -1,  hw, -hh, -hd,  0, 0, -1,
         hw,  hh, -hd,  0, 0, -1, -hw,  hh, -hd,  0, 0, -1,
        hw, -hh, -hd,  1, 0, 0,   hw, -hh,  hd,  1, 0, 0,
        hw,  hh,  hd,  1, 0, 0,   hw,  hh, -hd,  1, 0, 0,
        -hw, -hh, -hd, -1, 0, 0,  -hw, -hh,  hd, -1, 0, 0,
        -hw,  hh,  hd, -1, 0, 0,  -hw,  hh, -hd, -1, 0, 0,
        -1.0f, -0.5f, hd, 0, -0.5f, 0.5f,  1.0f, -0.5f, hd, 0, -0.5f, 0.5f,
         0, 0, hd + 3.0f, 0, 0, 1,
         1.0f, -0.5f, hd, 0.5f, 0, 0.5f,   1.0f, 0.5f, hd, 0.5f, 0, 0.5f,
         0, 0, hd + 3.0f, 0, 0, 1,
         1.0f, 0.5f, hd, 0, 0.5f, 0.5f,   -1.0f, 0.5f, hd, 0, 0.5f, 0.5f,
         0, 0, hd + 3.0f, 0, 0, 1,
        -1.0f, 0.5f, hd, -0.5f, 0, 0.5f,  -1.0f, -0.5f, hd, -0.5f, 0, 0.5f,
         0, 0, hd + 3.0f, 0, 0, 1,
    };

    std::vector<unsigned int> indices;
    for (int face = 0; face < 6; face++) {
        unsigned int base = face * 4;
        indices.push_back(base); indices.push_back(base + 1); indices.push_back(base + 2);
        indices.push_back(base); indices.push_back(base + 2); indices.push_back(base + 3);
    }
    for (int i = 0; i < 12; i++) indices.push_back(24 + i);

    m_indexCount = indices.size();

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void RoverRenderer::render(const RoverState& rover, const glm::vec3& color, bool selected,
                           bool engineRunning, const glm::mat4& view, const glm::mat4& projection) {
    m_shader.use();
    
    glm::mat4 model = rover.getModelMatrix();
    
    m_shader.setMat4("model", model);
    m_shader.setMat4("view", view);
    m_shader.setMat4("projection", projection);
    m_shader.setVec3("color", color);
    m_shader.setFloat("alpha", rover.online ? 1.0f : 0.5f);
    m_shader.setInt("selected", selected ? 1 : 0);
    m_shader.setInt("engineRunning", engineRunning ? 1 : 0);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

} // namespace terrafirma
