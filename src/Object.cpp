#include "Object.h"

// star::core::Object& star::core::Object::New(const std::string& filePath){
// }

star::core::Object::Object(std::unique_ptr<std::vector<common::Vertex>> vertexList, std::unique_ptr<std::vector<uint16_t>> indiciesList) :
	vertexList(std::move(vertexList)), 
	indiciesList(std::move(indiciesList))
{
}