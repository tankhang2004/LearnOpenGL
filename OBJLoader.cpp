#include "OBJLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

Mesh OBJLoader::LoadOBJ(const std::string& filepath) {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << filepath << std::endl;
        return {};
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "v") {
            // Vertex position
            glm::vec3 position;
            iss >> position.x >> position.y >> position.z;
            positions.push_back(position);
        }
        else if (prefix == "vt") {
            // Texture coordinate
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            texCoords.push_back(texCoord);
        }
        else if (prefix == "vn") {
            // Vertex normal
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (prefix == "f") {
            // Face - can be triangles or quads
            std::vector<std::string> faceTokens;
            std::string token;
            while (iss >> token) {
                faceTokens.push_back(token);
            }
            
            // Convert quads to triangles if necessary
            if (faceTokens.size() == 3) {
                // Triangle
                for (int i = 0; i < 3; i++) {
                    std::string faceToken = faceTokens[i];
                    std::istringstream tokenStream(faceToken);
                    std::string indexStr;
                    
                    Vertex vertex;
                    
                    // Parse vertex index
                    std::getline(tokenStream, indexStr, '/');
                    int vertexIndex = std::stoi(indexStr) - 1; // OBJ indices are 1-based
                    vertex.position = positions[vertexIndex];
                    
                    // Parse texture coordinate index
                    if (std::getline(tokenStream, indexStr, '/') && !indexStr.empty()) {
                        int texIndex = std::stoi(indexStr) - 1;
                        if (texIndex < texCoords.size()) {
                            vertex.texCoord = texCoords[texIndex];
                        }
                    }
                    
                    // Parse normal index
                    if (std::getline(tokenStream, indexStr) && !indexStr.empty()) {
                        int normalIndex = std::stoi(indexStr) - 1;
                        if (normalIndex < normals.size()) {
                            vertex.normal = normals[normalIndex];
                        }
                    }
                    
                    vertices.push_back(vertex);
                    indices.push_back(vertices.size() - 1);
                }
            }
            else if (faceTokens.size() == 4) {
                // Quad - split into two triangles
                std::vector<Vertex> quadVertices;
                
                for (int i = 0; i < 4; i++) {
                    std::string faceToken = faceTokens[i];
                    std::istringstream tokenStream(faceToken);
                    std::string indexStr;
                    
                    Vertex vertex;
                    
                    // Parse vertex index
                    std::getline(tokenStream, indexStr, '/');
                    int vertexIndex = std::stoi(indexStr) - 1;
                    vertex.position = positions[vertexIndex];
                    
                    // Parse texture coordinate index
                    if (std::getline(tokenStream, indexStr, '/') && !indexStr.empty()) {
                        int texIndex = std::stoi(indexStr) - 1;
                        if (texIndex < texCoords.size()) {
                            vertex.texCoord = texCoords[texIndex];
                        }
                    }
                    
                    // Parse normal index
                    if (std::getline(tokenStream, indexStr) && !indexStr.empty()) {
                        int normalIndex = std::stoi(indexStr) - 1;
                        if (normalIndex < normals.size()) {
                            vertex.normal = normals[normalIndex];
                        }
                    }
                    
                    quadVertices.push_back(vertex);
                }
                
                // First triangle: 0, 1, 2
                vertices.push_back(quadVertices[0]);
                indices.push_back(vertices.size() - 1);
                vertices.push_back(quadVertices[1]);
                indices.push_back(vertices.size() - 1);
                vertices.push_back(quadVertices[2]);
                indices.push_back(vertices.size() - 1);
                
                // Second triangle: 0, 2, 3
                vertices.push_back(quadVertices[0]);
                indices.push_back(vertices.size() - 1);
                vertices.push_back(quadVertices[2]);
                indices.push_back(vertices.size() - 1);
                vertices.push_back(quadVertices[3]);
                indices.push_back(vertices.size() - 1);
            }
        }
    }
    
    file.close();
    
    std::cout << "Loaded OBJ file: " << filepath << std::endl;
    std::cout << "Vertices: " << vertices.size() << std::endl;
    std::cout << "Indices: " << indices.size() << std::endl;
    std::cout << "Triangles: " << indices.size() / 3 << std::endl;
    
    Mesh mesh;
    mesh.vertices = vertices;
    mesh.indices = indices;
    return mesh;
}
