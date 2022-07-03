#include "Star_RenderObject.hpp"

namespace star{
namespace core {
	RenderObject::Builder& RenderObject::Builder::setNumFrames(size_t numImages) {
		this->numSwapChainImages = numImages; 
		return *this; 
	}

	RenderObject::Builder& RenderObject::Builder::addMesh(common::Mesh& mesh, common::Texture& texture) {
		this->meshInfos.push_back(std::make_pair<std::reference_wrapper<common::Mesh>, std::reference_wrapper<common::Texture>>(mesh, texture)); 
		return *this; 
	}

	std::unique_ptr<RenderObject> RenderObject::Builder::build() {
		return std::make_unique<RenderObject>(this->starDevice, this->gameObject, this->meshInfos, this->numSwapChainImages); 
	}

	RenderObject::RenderObject(StarDevice& starDevice, common::GameObject& gameObject, std::vector<std::pair<std::reference_wrapper<common::Mesh>, std::reference_wrapper<common::Texture>>> meshInfos,
		size_t numImages) 
		: starDevice(starDevice), objectHandle(objectHandle),
		gameObject(gameObject), uboDescriptorSets(numImages) {
		//create a texture for each mesh
		this->meshRenderInfos.resize(meshInfos.size());
		for (size_t i = 0; i < meshInfos.size(); i++) {
			auto& info = meshInfos.at(i); 
			this->meshRenderInfos.at(i) = std::make_unique<MeshRenderInfo>(info.first, std::make_unique<StarTexture>(this->starDevice, info.second));
		}
	}

	void render(vk::CommandBuffer& commandBuffer) {

	}

	common::Handle RenderObject::getHandle() {
		return this->objectHandle;
	}

	std::vector<vk::DescriptorSet>& RenderObject::getDefaultDescriptorSets() {
		return this->uboDescriptorSets;
	}
}
}