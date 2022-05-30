#include "Compiler.h"

std::vector<uint32_t> star::shadermanager::Compiler::Compile(const std::string& pathToFile, bool optimize){
    shaderc::Compiler compiler; 
    shaderc::CompileOptions options; 

    auto stageC = GetShaderCStageFlag(pathToFile); 
    auto name = common::FileHelpers::GetFileNameWithExtension(pathToFile); 

    // Like -DMY_DEFINE=1
    //options.AddMacroDefinition("MY_DEFINE", "1");
    if (optimize){
        options.SetOptimizationLevel(shaderc_optimization_level_size);
    }

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(pathToFile, stageC, name.c_str(), options); 

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << result.GetErrorMessage();
        return std::vector<uint32_t>(); 
    }

    return { result.cbegin(), result.cend() };
}

shaderc_shader_kind star::shadermanager::Compiler::GetShaderCStageFlag(const std::string& pathToFile){

   auto extension = common::FileHelpers::GetFileExtension(pathToFile); 

   if (extension == ".vert"){
       return shaderc_shader_kind::shaderc_vertex_shader; 
   } else if (extension == ".frag"){
       return shaderc_shader_kind::shaderc_fragment_shader; 
   }

   throw std::runtime_error("Compiler::GetShaderCStageFlag invalid shader stage"); 
    // if (vkShaderStage == vk::ShaderStageFlagBits::eVertex) {
    //         return shaderc_shader_kind::shaderc_vertex_shader; 
    //     }
    //     else if (vkShaderStage == vk::ShaderStageFlagBits::eFragment) {
    //         return shaderc_shader_kind::shaderc_fragment_shader; 
    //     }
    //     else {
    //         throw std::runtime_error("Invalid option -- ShadercHelper"); 
    //     }
    // }
}