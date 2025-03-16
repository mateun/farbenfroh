//
// Created by mgrus on 15.03.2025.
//

#ifndef COMPUTESHADER_H
#define COMPUTESHADER_H

enum class DispatchOutput {
    Texture,
    Buffer
};

class ComputeShader {
public:
    ComputeShader(const std::string & shaderPath);

    void initWithTexture(int width, int height);

    template<class T>
    void initWithShaderStorageBuffer(std::vector<T>& data);

    void bindSSBO();

    void dispatch(DispatchOutput dispatchOutput, glm::ivec3 groupSize) const;

    void setFloat(const std::string& uniformName, float time_passed);

private:
    // Different output options
    Texture * outputTexture = nullptr;
    GLuint handle = 0;
    GLuint ssbo = 0;
};



#endif //COMPUTESHADER_H
