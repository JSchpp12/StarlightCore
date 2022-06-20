#include "ObjectManager.h"

star::core::ObjectManager::~ObjectManager(){

}

star::common::Handle star::core::ObjectManager::Add(const std::string& pathToFile){
    //TODO: verify file type
	//TODO: have object manager register callbacks for objects -- if needed

	bool hasBeenLoaded = this->fileContainer.FileLoaded(pathToFile);
	if (!hasBeenLoaded){
		std::unique_ptr<std::vector<common::Vertex>> readVertexList(new std::vector<common::Vertex>);
		std::unique_ptr<std::vector<uint32_t>> readIndiciesList(new std::vector<uint32_t>);
		std::unique_ptr<common::GameObject> newObject(this->create(pathToFile)); 
		
		//record callbacks for object
		return this->fileContainer.AddFileResource(pathToFile, newObject); 
	}else{
		throw std::runtime_error("This object is already loaded"); 
	}
}

//star::common::Handle star::core::ObjectManager::Add(const std::string& pathToFile, common::Handle texture) {
//	bool hasBeenLoaded = this->fileContainer.FileLoaded(pathToFile); 
//
//	if (!hasBeenLoaded) {
//		std::unique_ptr<std::vector<common::Vertex>> readVertexList(new std::vector<common::Vertex>);
//		std::unique_ptr<std::vector<uint16_t>> readIndiciesList(new std::vector<uint16_t>);
//		std::unique_ptr<common::Object> newObject(this->create(pathToFile));
//		return this->fileContainer.AddFileResource(pathToFile, newObject);
//	}
//	else {
//		throw std::runtime_error("This object is already loaded"); 
//	}
//}

star::common::Handle star::core::ObjectManager::Add(const std::string& pathToFile, common::Handle texture, common::Handle vertShader, common::Handle fragShader, glm::vec3 scaleAmt) {
	bool hasBeenLoaded = this->fileContainer.FileLoaded(pathToFile); 

	if (!hasBeenLoaded) {
		std::unique_ptr<std::vector<common::Vertex>> readVertexList(new std::vector<common::Vertex>);
		std::unique_ptr<std::vector<uint32_t>> readIndiciesList(new std::vector<uint32_t>);

		//std::unique_ptr<common::Object> newObject(this->create(pathToFile, vertShader, fragShader, texture)); 
		std::unique_ptr<common::GameObject> newObject(this->create(pathToFile, scaleAmt, texture, vertShader, fragShader)); 
		return this->fileContainer.AddFileResource(pathToFile, newObject); 
	}
	else {
		throw std::runtime_error("This object is already loaded"); 
	}
}

void star::core::ObjectManager::load(const std::string& pathToFile, std::vector<common::Vertex>* vertexList, std::vector<uint32_t>* indiciesList) {
	/* Load Object From File */
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, pathToFile.c_str())) {
		throw std::runtime_error(warn + err);
	}

	float maxVal = 0;
	//need to scale object so that it fits on screen
	//combine all attributes into a single object 
	for (const auto& shape : shapes) {
		//tinyobj ensures three verticies per triangle  -- assuming unique verticies 
		for (const auto& index : shape.mesh.indices) {
			common::Vertex vertex{};

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
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]			//image needs to be flipped where 0 is the top (-1.0)
			};

			vertexList->push_back(vertex);
			indiciesList->push_back(indiciesList->size());
		}
	}

	//scale up factor 
	//maxVal *= 0.5;

	//SAME AS ABOVE 
	common::Vertex* currVertex;
	for (size_t i = 0; i < vertexList->size(); i++) {
		currVertex = &vertexList->at(i);
		currVertex->pos.x /= maxVal;
		currVertex->pos.y /= maxVal;
		currVertex->pos.z /= maxVal;
		currVertex->pos.z -= 0.7;
	}

	std::cout << "Loaded: " << pathToFile << std::endl; 
}

star::common::GameObject* star::core::ObjectManager::create(const std::string& pathToFile, glm::vec3 scaleAmt, common::Handle texture, common::Handle vertShader, common::Handle fragShader) {
	std::unique_ptr<std::vector<common::Vertex>> readVertexList(new std::vector<common::Vertex>);
	std::unique_ptr<std::vector<uint32_t>> readIndiciesList(new std::vector<uint32_t>);

	//load object 
	this->load(pathToFile, readVertexList.get(), readIndiciesList.get());
	return new common::GameObject(std::move(readVertexList), std::move(readIndiciesList), scaleAmt, vertShader, fragShader, texture);
}