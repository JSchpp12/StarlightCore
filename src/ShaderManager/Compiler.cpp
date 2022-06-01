#include "Compiler.h"

std::vector<uint32_t> star::core::Compiler::Compile(const std::string& pathToFile, bool optimize){
    shaderc::Compiler shaderCompiler; 
    shaderc::CompileOptions compilerOptions; 

    auto stageC = GetShaderCStageFlag(pathToFile); 
    auto name = common::FileHelpers::GetFileNameWithExtension(pathToFile); 
    auto fileCode = common::FileHelpers::ReadFile(pathToFile, true); 

    std::string preprocessed = PreprocessShader(name, stageC, fileCode.c_str()); 

    shaderc::SpvCompilationResult compileResult = shaderCompiler.CompileGlslToSpv(preprocessed.c_str(), stageC, name.c_str(), compilerOptions); 

    if (compileResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << compileResult.GetErrorMessage();
        return std::vector<uint32_t>(); 
    }
    return { compileResult.cbegin(), compileResult.cend() };
}

shaderc_shader_kind star::core::Compiler::GetShaderCStageFlag(const std::string& pathToFile){

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

std::string star::core::Compiler::PreprocessShader(const std::string& sourceName, shaderc_shader_kind stage, const std::string& source){
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    // Like -DMY_DEFINE=1
    options.AddMacroDefinition("MY_DEFINE", "1");

    shaderc::PreprocessedSourceCompilationResult result =
        compiler.PreprocessGlsl(source, stage, sourceName.c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << result.GetErrorMessage();
        return "";
    }

    return { result.cbegin(), result.cend() };
}