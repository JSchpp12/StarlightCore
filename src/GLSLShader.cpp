#include "GLSLShader.h"

namespace star::core {
    std::unique_ptr<common::Shader> GLSLShader::load(const std::string& pathToFile) {
        auto compilationResult = std::make_unique<std::vector<uint32_t>>(std::move(Compiler::Compile(pathToFile, true)));
        return std::make_unique<common::Shader>(std::move(compilationResult));
    }
}