#include "Object.h"

namespace star::core {
	Object::Object(std::unique_ptr<std::vector<common::Vertex>> vertexList, std::unique_ptr<std::vector<uint16_t>> indiciesList) :
		vertexList(std::move(vertexList)),
		indiciesList(std::move(indiciesList)) { }
}