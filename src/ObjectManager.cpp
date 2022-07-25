#include "ObjectManager.hpp"

namespace star::core {
	void ObjectManager::loadObject(const std::string& pathToFile, std::vector<common::Vertex>* vertexList, std::vector<uint32_t>* indiciesList) {
		/* Load Object From File */
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		//get base directory
		auto baseDir = common::FileHelpers::GetBaseFileDirectory(pathToFile);

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, pathToFile.c_str(), baseDir.c_str())) {
			throw std::runtime_error(warn + err);
		}

		float maxVal = 0;
		//need to scale object so that it fits on screen
		//combine all attributes into a single object 
		for (const auto& shape : shapes) {
			//tinyobj ensures three verticies per triangle  -- assuming unique verticies 
			for (const auto& index : shape.mesh.indices) {
				common::Vertex vertex{};
				if (index.vertex_index >= 0) {
					vertex.pos = {
						//must multiply index by 3 due to object being type float rather than glm::vec3 
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2],
					};
				}

				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2],
					};
				}

				//TODO: scaling very bad...switch to method of moving camera 
				if (vertex.pos.x > maxVal) {
					maxVal = vertex.pos.x;
				}
				if (vertex.pos.y > maxVal) {
					maxVal = vertex.pos.y;
				}
				if (vertex.pos.z > maxVal) {
					maxVal = vertex.pos.z;
				}

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]			//image needs to be flipped where 0 is the top (-1.0)
				};

				vertexList->push_back(vertex);
				indiciesList->push_back(indiciesList->size());
			}
		}
		common::Vertex* currVertex;
		for (size_t i = 0; i < vertexList->size(); i++) {
			currVertex = &vertexList->at(i);
			currVertex->pos.x /= maxVal;
			currVertex->pos.y /= maxVal;
			currVertex->pos.z /= maxVal;
		}
		std::cout << "Loaded: " << pathToFile << std::endl;
	}

	common::Handle ObjectManager::createAppropriateHandle() {
		common::Handle newHandle; 
		newHandle.type = common::Handle_Type::object; 
		return newHandle; 
	}
}