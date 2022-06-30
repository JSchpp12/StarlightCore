#include "ObjectManager.hpp"


star::core::ObjectManager::~ObjectManager(){

}

star::common::Handle star::core::ObjectManager::add(const std::string& pathToFile){
    //TODO: verify file type
	//TODO: have object manager register callbacks for objects -- if needed
	throw std::runtime_error("This is deprecated");

	bool hasBeenLoaded = this->fileContainer.fileLoaded(pathToFile);
	if (!hasBeenLoaded){
		std::unique_ptr<std::vector<common::Vertex>> readVertexList(new std::vector<common::Vertex>);
		std::unique_ptr<std::vector<uint32_t>> readIndiciesList(new std::vector<uint32_t>);
		std::unique_ptr<common::GameObject> newObject(this->create(pathToFile, nullptr)); 
		
		common::Handle newHandle;
		newHandle.type = common::Handle_Type::object;
		//record callbacks for object
		this->addResource(pathToFile, std::move(newObject), newHandle); 
		return newHandle; 
	}else{
		throw std::runtime_error("This object is already loaded"); 
	}
}

star::common::Handle star::core::ObjectManager::Add(const std::string& pathToFile, common::Material* material, glm::vec3 position, glm::vec3 scaleAmt, common::Handle texture, common::Handle vertShader, common::Handle fragShader) {
	bool hasBeenLoaded = false; 
	if (!hasBeenLoaded) {
		std::unique_ptr<std::vector<common::Vertex>> readVertexList(new std::vector<common::Vertex>);
		std::unique_ptr<std::vector<uint32_t>> readIndiciesList(new std::vector<uint32_t>);

		std::unique_ptr<common::GameObject> newObject(this->create(pathToFile, material, position, scaleAmt, texture, vertShader, fragShader));
		common::Handle newHandle; 
		newHandle.type = common::Handle_Type::object; 
		this->addResource(pathToFile, std::move(newObject), newHandle);
		return newHandle; 
	}
	else {
		throw std::runtime_error("This object is already loaded"); 
	}
}

star::common::Handle star::core::ObjectManager::Add(std::unique_ptr<std::vector<common::Vertex>> verticies, std::unique_ptr<std::vector<uint32_t>> indicies,
	glm::vec3 position, glm::vec3 scaleAmt, common::Material* material,
	common::Handle texture, common::Handle vertShader,
	common::Handle fragShader) {

	std::unique_ptr<common::GameObject> newObject = std::make_unique<common::GameObject>(std::move(verticies), std::move(indicies), position, scaleAmt, material, texture, vertShader, fragShader);
	common::Handle newHandle; 
	newHandle.type = common::Handle_Type::object;
	this->addResource(std::move(newObject), newHandle); 
	return newHandle; 
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

star::common::GameObject* star::core::ObjectManager::create(const std::string& pathToFile, common::Material* material, glm::vec3 position, glm::vec3 scaleAmt, common::Handle texture, common::Handle vertShader, common::Handle fragShader) {
	std::unique_ptr<std::vector<common::Vertex>> readVertexList(new std::vector<common::Vertex>);
	std::unique_ptr<std::vector<uint32_t>> readIndiciesList(new std::vector<uint32_t>);

	//load object 
	this->load(pathToFile, readVertexList.get(), readIndiciesList.get());
	return new common::GameObject(std::move(readVertexList), std::move(readIndiciesList), position, scaleAmt, material, vertShader, fragShader, texture);
}