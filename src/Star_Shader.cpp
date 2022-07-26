#include "Star_Shader.h"

namespace star::core {
    Shader::Shader(const common::Shader& shader) : compiledCode(load(shader.path)), common::Shader(shader) { }

    std::unique_ptr<std::vector<uint32_t>> Shader::load(const std::string& pathToFile) {
        return Compiler::compile(pathToFile, true);
    }
}