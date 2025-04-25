//
// Created by mgrus on 23.04.2025.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <Windows.h>
#include <symbol_exports.h>
#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>


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

    template<typename T>
    struct VertexFormatFor;

    template<> struct VertexFormatFor<float>        { static constexpr auto value = VertexAttributeFormat::Float; };
    template<> struct VertexFormatFor<glm::vec2>    { static constexpr auto value = VertexAttributeFormat::Float2; };
    template<> struct VertexFormatFor<glm::vec3>    { static constexpr auto value = VertexAttributeFormat::Float3; };
    template<> struct VertexFormatFor<glm::vec4>    { static constexpr auto value = VertexAttributeFormat::Float4; };

    template<typename T>
    struct ComponentsFor;

    template<> struct ComponentsFor<float> { static constexpr auto value = 1; };
    template<> struct ComponentsFor<glm::vec2> { static constexpr auto value = 2; };
    template<> struct ComponentsFor<glm::vec3> { static constexpr auto value = 3; };
    template<> struct ComponentsFor<glm::vec4> { static constexpr auto value = 4; };


    struct VertexAttribute {
        VertexAttributeSemantic semantic;  // What it represents (Position, UV, etc.)
        VertexAttributeFormat format;      // Data format (float3, uint4, etc.)
        uint32_t location;                 // Shader binding location (e.g., layout(location = 0))
        uint32_t offset;                   // Byte offset within vertex
        size_t components;                  // how many components (e.g. 3 for glm::vec3)
        size_t stride;                      // byte stride between vertices


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
        uint32_t id;

    };

    struct FontHandle {
        uint32_t id;
        TextureHandle atlasTexture;
    };




    /**
     * Allows to compose a modular fragment shader and
     * retrieve its source code for reuse.
     */
    class FragmentShaderBuilder {
        public:
            virtual ~FragmentShaderBuilder() = default;
            virtual FragmentShaderBuilder& color() = 0;
            virtual FragmentShaderBuilder& textRender() = 0;
            virtual FragmentShaderBuilder& diffuseTexture(uint8_t textureUnit = 0) = 0;
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

        virtual VertexShaderBuilder& position(uint8_t slot) = 0;
        virtual VertexShaderBuilder& normal(uint8_t slot) = 0;
        virtual VertexShaderBuilder& uv(uint8_t slot) = 0;
        virtual VertexShaderBuilder& mvp() = 0;
        virtual VertexShaderBuilder& worldMatrix() = 0;
        virtual VertexShaderBuilder& projectionMatrix() = 0;
        virtual VertexShaderBuilder& viewMatrix() = 0;
        virtual std::string build() const = 0;


    };

    class VertexBufferBuilder {
    public:

        virtual VertexBufferBuilder& attributeVec3(VertexAttributeSemantic semantic, const std::vector<glm::vec3>& data) = 0;
        virtual VertexBufferBuilder& attributeVec2(VertexAttributeSemantic semantic, const std::vector<glm::vec2>& data) = 0;
        virtual VertexBufferHandle build() const = 0;

    };

    struct VertexBufferCreateInfo  {
        std::vector<float> data;
        size_t element_size = 0;
        size_t stride = 0;
    };

    struct RenderTarget {
        uint32_t id;
        TextureHandle colorTex;
        uint32_t depthRbo;
        uint32_t stencilRbo;
        int width, height;

    };

    class RenderTargetBuilder {
    public:
        virtual RenderTargetBuilder& size(int w, int h) = 0;
        virtual RenderTargetBuilder& color() = 0;
        virtual RenderTargetBuilder& depth() = 0;
        virtual RenderTargetBuilder& stencil() = 0;
        virtual RenderTarget build() = 0;
    };

    struct Image {
        uint8_t* pixels = nullptr;
        int width = 0;
        int height = 0;
        int channels = 4; // RGBA by default

        size_t sizeInBytes() const {
            return width * height * channels;
        }
    };

    enum class TextureFormat {
        // Color formats
        R8,         // 8-bit Red
        RG8,        // 8-bit Red + Green
        RGB8,       // 8-bit RGB
        RGBA8,      // 8-bit RGBA

        // sRGB formats (gamma-corrected color)
        SRGB8,      // sRGB RGB
        SRGBA8,     // sRGB RGBA

        // High precision float formats
        R16F,
        RG16F,
        RGB16F,
        RGBA16F,

        R32F,
        RG32F,
        RGB32F,
        RGBA32F,

        // Depth / stencil formats
        Depth16,
        Depth24,
        Depth32F,
        Depth24Stencil8,
        Depth32FStencil8,

        // Integer formats (rare)
        R32I,
        RG32I,
        RGBA32I,

        Unknown, // fallback
    };


    ENGINE_API void  beginFrame();
    ENGINE_API void  endFrame();
    ENGINE_API void  present(HDC hdc);

    ENGINE_API void  setClearColor(float r, float g, float b, float a);
    ENGINE_API void  clear();

    ENGINE_API void  setViewport(int x, int y, int width, int height);

    // Geometry
    typedef std::unique_ptr<VertexBufferBuilder> (*VertexBufferBuilderFn)(void);
    ENGINE_API void registerVertexBufferBuilder(VertexBufferBuilderFn fn);
    ENGINE_API std::unique_ptr<VertexBufferBuilder> vertexBufferBuilder();
    ENGINE_API VertexBufferHandle createVertexBuffer(VertexBufferCreateInfo create_info);

    typedef IndexBufferHandle (*CreateIndexBufferFn)(std::vector<uint32_t> data);
    ENGINE_API void registerCreateIndexBuffer(CreateIndexBufferFn fn);
    ENGINE_API IndexBufferHandle createIndexBuffer(std::vector<uint32_t> data);

    typedef Mesh (*CreateMeshFn)(VertexBufferHandle vbo, IndexBufferHandle ibo, const std::vector<VertexAttribute> & attributes, size_t index_count);
    ENGINE_API void registerCreateMesh(CreateMeshFn fn);
    ENGINE_API Mesh createMesh(VertexBufferHandle vbo, IndexBufferHandle ibo, const std::vector<VertexAttribute> & attributes, size_t index_count);


    // Shader
    ENGINE_API std::unique_ptr<VertexShaderBuilder> vertexShaderBuilder();
    ENGINE_API std::unique_ptr<FragmentShaderBuilder> fragmentShaderBuilder();
    ENGINE_API ShaderHandle compileVertexShader(const std::string& source);
    ENGINE_API ShaderHandle compileFragmentShader(const std::string& source);
    ENGINE_API ProgramHandle linkShaderProgram(ShaderHandle vertexShader, ShaderHandle fragmentShader);
    ENGINE_API ProgramHandle  createShaderProgram(const char* vertexShader, const char* fragmentShader);
    ENGINE_API void bindShader(ShaderHandle shader);
    ENGINE_API void bindProgram(ProgramHandle prog);

    template<typename T>
    ENGINE_API bool setShaderValue(ProgramHandle program, const std::string& name, const T& value);


    // Texture
    ENGINE_API Image createImageFromFile(const std::string& filename);
    typedef TextureHandle (*CreateTextureFn)(const Image&, TextureFormat);
    ENGINE_API void registerCreateTexture(CreateTextureFn fn);
    ENGINE_API TextureHandle createTexture(const Image& image, TextureFormat format = TextureFormat::SRGBA8);
    ENGINE_API void bindTexture(TextureHandle texture);

    // Rendertargets
    ENGINE_API std::unique_ptr<RenderTargetBuilder> renderTargetBuilder();
    ENGINE_API void bindRenderTarget(const RenderTarget& renderTarget);
    ENGINE_API void bindDefaultRenderTarget();

    // Font
    ENGINE_API FontHandle createFontFromFile(const std::string& fileToTTF, float fontSize);

    // Drawing geometry
    ENGINE_API void drawMesh(Mesh m);
    ENGINE_API Mesh drawTextIntoQuad(FontHandle font, const std::string& text);


}

#endif //RENDERER_H
