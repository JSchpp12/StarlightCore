#include "StarSystem_RenderPointLight.hpp"

namespace star {
namespace core {

	RenderSysPointLight::~RenderSysPointLight() {
		this->RenderSysObj::~RenderSysObj(); 
	}

	void RenderSysPointLight::addLight(common::Light* newLight, common::GameObject* linkedObject, size_t numSwapChainImages) {
		this->lightList.push_back(newLight); 

		this->RenderSysObj::addObject(newLight->getLinkedObjectHandle(), linkedObject, numSwapChainImages);
	}
}
}