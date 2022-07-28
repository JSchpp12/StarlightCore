/* Date: 07/27/2022
* This class is the core of the rendering operations that the engine will call. As it is responsible
* for the objects to draw and binding the correct descriptor sets for each object. There will be one of these for 
* each shader program combination.
*/

#pragma once 
#include "SC/Handle.hpp"
#include "Star_RenderObject.hpp"

#include <vector>
#include <memory>

namespace star::core{
	class RenderSystem {
	public:
		RenderSystem(std::vector<common::GameObject>& gameObjects);

	protected:

	private:


	};
}