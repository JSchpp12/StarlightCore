#include "GLSLShader.h"

namespace star::core {
    GLSLShader* GLSLShader::New(const std::string& pathToFile) {
        GLSLShader* newShader;

        auto compilationResult = std::make_unique<std::vector<uint32_t>>(Compiler::Compile(pathToFile, true));

        newShader = new GLSLShader(std::move(compilationResult));

        return newShader;
    }

    GLSLShader::GLSLShader(std::unique_ptr<std::vector<uint32_t>> compiledCode) : Shader(std::move(compiledCode)) {}
}