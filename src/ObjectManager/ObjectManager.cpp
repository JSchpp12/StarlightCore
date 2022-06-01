#include "ObjectManager.h"

star::common::Handle star::core::ObjectManager::Add(const std::string& pathToFile){
    //TODO: verify file type

	bool hasBeenLoaded = this->fileContainer.FileLoaded(pathToFile);
	if (!hasBeenLoaded){
		std::unique_ptr<std::vector<Vertex>> readVertexList(new std::vector<Vertex>); 
		std::unique_ptr<std::vector<uint16_t>> readIndiciesList(new std::vector<uint16_t>); 

		/* Load Object From File */ 
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, pathToFile.c_str())) {
			throw std::runtime_error(warn + err);
		}

		//need to scale object so that it fits on screen
		
		float maxVal = 0; 

		//combine all attributes into a single object 
		for (const auto& shape : shapes) {
			//tinyobj ensures three verticies per triangle  -- assuming unique verticies 
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				vertex.pos = {
					//must multiply index by 3 due to object being type float rather than glm::vec3 
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

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
					attrib.texcoords[2 * index.texcoord_index + 1]
				};

				readVertexList->push_back(vertex);
				readIndiciesList->push_back(readIndiciesList->size());
			}
		}

		//scale up factor 
		maxVal *= 0.5; 

		//SAME AS ABOVE 
		Vertex* currVertex; 
		for (size_t i = 0; i < readVertexList->size(); i++){
			currVertex = &readVertexList->at(i); 
			currVertex->pos.x /= maxVal; 
			currVertex->pos.y /= maxVal; 
			currVertex->pos.z /= maxVal; 
			currVertex->pos.z -= 0.7;
		}

		std::unique_ptr<Object> newObject(new Object(std::move(readVertexList), std::move(readIndiciesList))); 

		return this->fileContainer.AddFileResource(pathToFile, newObject); 
	}else{
		throw std::runtime_error("This object is already loaded"); 
	}
}