//
// Created by mgrus on 23.04.2025.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <symbol_exports.h>
#include <vector>
#include <string>
#include <memory>

namespace renderer {

    struct ShaderHandle {
        uint32_t id;
    };

    struct ProgramHandle {
        uint32_t id;
    };

    struct TextureHandle {
        uint32_t id;
    };

    struct VertexBufferHandle {
        uint32_t id;
    };

    struct IndexBufferHandle {
        uint32_t id;
    };

    enum class VertexAttributeSemantic {
        Position,
        Normal,
        Tangent,
        UV0,
        UV1,
        Color,
        BoneWeights,
        BoneIndices,
        Custom0,
        Custom1,
        // etc...
    };

    enum class VertexAttributeFormat {
        Float,       // 1x float
        Float2,      // vec2
        Float3,      // vec3
        Float4,      // vec4
        UInt,
        UInt2,
        UInt4,
        // more formats as needed (short, byte, normalized, etc.)
    };


    struct VertexAttribute {
        VertexAttributeSemantic semantic;  // What it represents (Position, UV, etc.)
        VertexAttributeFormat format;      // Data format (float3, uint4, etc.)
        uint32_t location;                 // Shader binding location (e.g., layout(location = 0))
        uint32_t offset;                   // Byte offset within vertex
    };

    struct VertexLayout {
        // Attribute bindings (e.g. position = 0, normal = 1, etc.)
        std::vector<VertexAttribute> attributes;
    };

    enum class PrimitiveTopology {
        Triangle_List,
        Lines,

    };

    struct Mesh {
        VertexBufferHandle vertex_buffer;
        IndexBufferHandle index_buffer;
        VertexLayout layout;
        size_t index_count;
        PrimitiveTopology primitive_topology;

    };

    class FragmentShaderBuilder {
        public:
        virtual ~FragmentShaderBuilder() = default;
        virtual FragmentShaderBuilder& color() = 0;
        virtual std::string build() const = 0;
    };

    /**
     * This builder pattern class allows for
     * composing modulare vertex shaders with different attributes.
     * build() creates the actual source code of the shader
     * which can then be written to disk or used in a compilation step etc.
     *
     */
    class VertexShaderBuilder {
    public:
        virtual ~VertexShaderBuilder() = default;

        virtual VertexShaderBuilder& position() = 0;
        virtual VertexShaderBuilder& normal() = 0;
        virtual VertexShaderBuilder& uv() = 0;
        virtual std::string build() const = 0;
    };

    ENGINE_API void  beginFrame();
    ENGINE_API void  endFrame();
    ENGINE_API void  present(HDC hdc);

    ENGINE_API void  setClearColor(float r, float g, float b, float a);
    ENGINE_API void  clear();

    ENGINE_API void  setViewport(int x, int y, int width, int height);

    // Shader
    ENGINE_API std::unique_ptr<VertexShaderBuilder>  vertexShaderBuilder();
    ENGINE_API std::unique_ptr<FragmentShaderBuilder>  fragmentShaderBuilder();
    ENGINE_API ShaderHandle compileVertexShader(const std::string& source);
    ENGINE_API ShaderHandle compileFragmentShader(const std::string& source);
    ENGINE_API ProgramHandle linkShaderProgram(ShaderHandle vertexShader, ShaderHandle fragmentShader);
    ENGINE_API ProgramHandle  createShaderProgram(const char* vertexShader, const char* fragmentShader);
    ENGINE_API void bindShader(ShaderHandle shader);

    // Texture
    ENGINE_API TextureHandle  createTexture(const char* texturePath);
    ENGINE_API void bindTexture(TextureHandle texture);


}

#endif //RENDERER_H
