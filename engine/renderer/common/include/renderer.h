//
// Created by mgrus on 23.04.2025.
//

#ifndef RENDERER_H
#define RENDERER_H
#include <Windows.h>
#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>

struct Skeleton;
struct SkeletalAnimation;
struct Joint;

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
        Float, // 1x float
        Float2, // vec2
        Float3, // vec3
        Float4, // vec4
        UInt,
        UInt2,
        UInt4,
        // more formats as needed (short, byte, normalized, etc.)
    };

    template<typename T>
    struct VertexFormatFor;

    template<>
    struct VertexFormatFor<float> {
        static constexpr auto value = VertexAttributeFormat::Float;
    };

    template<>
    struct VertexFormatFor<glm::vec2> {
        static constexpr auto value = VertexAttributeFormat::Float2;
    };

    template<>
    struct VertexFormatFor<glm::vec3> {
        static constexpr auto value = VertexAttributeFormat::Float3;
    };

    template<>
    struct VertexFormatFor<glm::vec4> {
        static constexpr auto value = VertexAttributeFormat::Float4;
    };

    template<typename T>
    struct ComponentsFor;

    template<>
    struct ComponentsFor<float> {
        static constexpr auto value = 1;
    };

    template<>
    struct ComponentsFor<glm::vec2> {
        static constexpr auto value = 2;
    };

    template<>
    struct ComponentsFor<glm::vec3> {
        static constexpr auto value = 3;
    };

    template<>
    struct ComponentsFor<glm::vec4> {
        static constexpr auto value = 4;
    };


    struct VertexAttribute {
        VertexAttributeSemantic semantic; // What it represents (Position, UV, etc.)
        VertexAttributeFormat format; // Data format (float3, uint4, etc.)
        uint32_t location; // Shader binding location (e.g., layout(location = 0))
        uint32_t offset; // Byte offset within vertex
        size_t components; // how many components (e.g. 3 for glm::vec3)
        size_t stride; // byte stride between vertices
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
        std::vector<glm::vec4> joint_indices;
        std::vector<glm::vec4> joint_weights;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        Skeleton* skeleton = nullptr;

        // This is the overall vertex data, e.g. position and uvs packed tightly, array of structs.
        std::vector<float> vertex_data;
        std::vector<SkeletalAnimation *> animations;
    };





    /**
     * Allows to compose a modular fragment shader and
     * retrieve its source code for reuse.
     */
    class FragmentShaderBuilder {
    public:
        virtual ~FragmentShaderBuilder() = default;

        virtual FragmentShaderBuilder &color() = 0;

        virtual FragmentShaderBuilder &textRender() = 0;

        virtual FragmentShaderBuilder &diffuseTexture(uint8_t textureUnit = 0, bool flipUVs = false) = 0;

        virtual std::string build() const = 0;
    };


    /**
     * Implemetns the FramgementShaderBuilder interface for OpenGL GLSL >= 4.6.
     */
    class GLSLFramgentShaderBuilder : public renderer::FragmentShaderBuilder {
    public:
        FragmentShaderBuilder &color() override;

        FragmentShaderBuilder &diffuseTexture(uint8_t textureUnit, bool flipUVs) override;

        FragmentShaderBuilder &textRender() override;

        std::string build() const override;

    private:
        bool useTextRender = false;
        bool useColor = false;
        bool useDiffuseTexture = false;
        uint8_t diffuseTextureUnit = 0;
        bool flipUVs = false;
    };

    struct CustomUniform {
        std::string declaration;
        std::string custom_code;
    };

    struct CustomUniformBufferObject {
        std::string struct_name = "__undefined_struct_name";
        int set = 0;
        int binding = 0;
        std::vector<CustomUniform> nested_uniforms;
        std::string name = "___undefined_ubo_name___";
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

        virtual VertexShaderBuilder &position(uint8_t slot) = 0;

        virtual VertexShaderBuilder &normal(uint8_t slot) = 0;

        virtual VertexShaderBuilder &uv(uint8_t slot) = 0;

        virtual VertexShaderBuilder &mvp() = 0;

        virtual VertexShaderBuilder &worldMatrix() = 0;

        virtual VertexShaderBuilder &projectionMatrix() = 0;

        virtual VertexShaderBuilder &viewMatrix() = 0;

        virtual VertexShaderBuilder &uniform(CustomUniform) = 0;

        virtual VertexShaderBuilder &uniformBufferObject(CustomUniformBufferObject) = 0;

        virtual std::string build() const = 0;
    };


    /**
     * Implements the VertexSahderBuilder interface for GLSL (GL Version >= 4.6).
     */
    class GLSLVertexShaderBuilder : public VertexShaderBuilder {
    public:
        GLSLVertexShaderBuilder &position(uint8_t slot) override;

        GLSLVertexShaderBuilder &normal(uint8_t slot) override;

        GLSLVertexShaderBuilder &uv(uint8_t slot) override;

        VertexShaderBuilder &mvp() override;


        VertexShaderBuilder &worldMatrix() override;

        VertexShaderBuilder &projectionMatrix() override;

        VertexShaderBuilder &viewMatrix() override;

        GLSLVertexShaderBuilder &uniform(CustomUniform customUniform) override;

        VertexShaderBuilder &uniformBufferObject(CustomUniformBufferObject) override;

        [[nodiscard]] std::string build() const override;

    private:
        bool hasPosition = false;
        bool hasNormal = false;
        bool hasUV = false;
        uint8_t positionSlot = 0;
        uint8_t normalSlot = 0;
        uint8_t uvSlot = 0;
        bool hasMVPUniforms = false;
        bool hasWorldMatrixUniform;
        bool hasProjectionMatrixUniform;
        bool hasViewMatrixUniform;
        std::vector<CustomUniform> custom_uniforms;
        std::vector<CustomUniformBufferObject> custom_uniform_buffer_objects;
    };

    class VertexBufferBuilder {
    public:
        virtual VertexBufferBuilder &attributeVec3(VertexAttributeSemantic semantic, const std::vector<glm::vec3> &data)
        = 0;

        virtual VertexBufferBuilder &attributeVec2(VertexAttributeSemantic semantic, const std::vector<glm::vec2> &data)
        = 0;

        virtual VertexBufferHandle build() const = 0;

        virtual void update(VertexBufferHandle existingVBO) const = 0;
    };

    struct VertexBufferCreateInfo {
        std::vector<float> data;
        size_t element_size = 0;
        size_t stride = 0;
    };

    struct VertexBufferUpdateInfo {
        std::vector<float> data;
        size_t element_size = 0;
        size_t stride = 0;
        VertexBufferHandle oldVBO;
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
        virtual RenderTargetBuilder &size(int w, int h) = 0;

        virtual RenderTargetBuilder &color() = 0;

        virtual RenderTargetBuilder &depth() = 0;

        virtual RenderTargetBuilder &stencil() = 0;

        virtual RenderTarget build() = 0;
    };

    struct BoundingBox {
        float left;
        float top;
        float right;
        float bottom;
    };

    struct Image {
        uint8_t *pixels = nullptr;
        int width = 0;
        int height = 0;
        int channels = 4; // RGBA by default

        size_t sizeInBytes() const {
            return width * height * channels;
        }
    };

    struct FontHandle {
        uint32_t id;
        TextureHandle atlasTexture;
        Image atlasImage;
    };

    enum class IndexFormat {
        UInt8,
        UInt16,
        UInt32
    };

    struct IndexBufferDesc {
        const void *data;
        size_t size_in_bytes;
        unsigned int format;
    };

    enum class TextureFormat {
        // Color formats
        R8, // 8-bit Red
        RG8, // 8-bit Red + Green
        RGB8, // 8-bit RGB
        RGBA8, // 8-bit RGBA

        // sRGB formats (gamma-corrected color)
        SRGB8, // sRGB RGB
        SRGBA8, // sRGB RGBA

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

    void beginFrame();

    void endFrame();

    void present(HDC hdc);

    void setClearColor(float r, float g, float b, float a);

    void clear();

    void setViewport(int x, int y, int width, int height);

    // Geometry
    typedef std::unique_ptr<VertexBufferBuilder> (*VertexBufferBuilderFn)(void);

    void registerVertexBufferBuilder(VertexBufferBuilderFn fn);

    std::unique_ptr<VertexBufferBuilder> vertexBufferBuilder();


    typedef VertexBufferHandle (*CreateVertexBufferFn)(VertexBufferCreateInfo create_info);
    void registerCreateVertexBuffer(CreateVertexBufferFn fn);
    VertexBufferHandle createVertexBuffer(VertexBufferCreateInfo create_info);

    void updateVertexBuffer(VertexBufferUpdateInfo updateInfo);

    typedef IndexBufferHandle (*CreateIndexBufferFn)(const IndexBufferDesc &);
    void registerCreateIndexBuffer(CreateIndexBufferFn fn);
    IndexBufferHandle createIndexBuffer(const IndexBufferDesc &);

    typedef void (*UpdateIndexBufferFn)(IndexBufferHandle iboHandle, std::vector<uint32_t> data);

    void registerUpdateIndexBuffer(UpdateIndexBufferFn fn);

    void updateIndexBuffer(IndexBufferHandle iboHandle, std::vector<uint32_t> data);

    typedef Mesh (*CreateMeshFn)(VertexBufferHandle vbo, IndexBufferHandle ibo,
                                 const std::vector<VertexAttribute> &attributes, size_t index_count);

    void registerCreateMesh(CreateMeshFn fn);

    Mesh createMesh(VertexBufferHandle vbo, IndexBufferHandle ibo, const std::vector<VertexAttribute> &attributes,
                    size_t index_count);

    typedef Mesh (*ImportMeshFn)(const std::string &filename);

    void registerImportMesh(ImportMeshFn fn);

    Mesh importMesh(const std::string &filename);

    Mesh parseGLTF(const std::string &filename);

    // Shader
    std::unique_ptr<VertexShaderBuilder> vertexShaderBuilder();

    std::unique_ptr<FragmentShaderBuilder> fragmentShaderBuilder();

    ShaderHandle compileVertexShader(const std::string &source);

    ShaderHandle compileFragmentShader(const std::string &source);

    ProgramHandle linkShaderProgram(ShaderHandle vertexShader, ShaderHandle fragmentShader);

    ProgramHandle createShaderProgram(const char *vertexShader, const char *fragmentShader);

    void bindShader(ShaderHandle shader);

    void bindProgram(ProgramHandle prog);

    template<typename T>
    bool setShaderValue(ProgramHandle program, const std::string &name, const T &value);


    // Texture
    Image createImageFromFile(const std::string &filename);

    typedef TextureHandle (*CreateTextureFn)(const Image &, TextureFormat);

    void registerCreateTexture(CreateTextureFn fn);

    TextureHandle createTexture(const Image &image, TextureFormat format = TextureFormat::SRGBA8);

    void bindTexture(TextureHandle texture);

    // Rendertargets
    std::unique_ptr<RenderTargetBuilder> renderTargetBuilder();

    void bindRenderTarget(const RenderTarget &renderTarget);

    void bindDefaultRenderTarget();

    // Font
    FontHandle createFontFromFile(const std::string &fileToTTF, float fontSize);

    // Drawing geometry
    void drawMesh(Mesh m, const std::string &debugInfo = "");

    void drawTextIntoQuadGeometry(FontHandle fontHandle, const std::string& text,
        std::vector<glm::vec3>& outPositions, std::vector<glm::vec2>& outUVs, std::vector<uint32_t>& outIndices);

    typedef Mesh (*DrawTextIntoQuadFn)(FontHandle fontHandle, const std::string &);
    void registerDrawTextIntoQuad(DrawTextIntoQuadFn fn);
    Mesh drawTextIntoQuad(FontHandle font, const std::string &text);

    BoundingBox measureText(FontHandle fontHandle, const std::string &text);

    void updateText(Mesh &mesh, FontHandle font, const std::string &newText);


    typedef void* (*GetVertexBufferMemoryForHandleFn)(VertexBufferHandle vbh);
    void registerGetVertexBufferMemoryForHandle(GetVertexBufferMemoryForHandleFn fn);
    void* getVertexBufferMemoryForHandle(VertexBufferHandle vbh);

    typedef void* (*GetVertexBufferForHandleFn)(VertexBufferHandle vbh);
    void registerGetVertexBufferForHandle(GetVertexBufferForHandleFn fn);
    void* getVertexBufferForHandle(VertexBufferHandle vbh);

    typedef void* (*GetNativeIndexBufferStructForHandleFn)(IndexBufferHandle ibh);
    void registerGetNativeIndexBufferStructForHandle(GetNativeIndexBufferStructForHandleFn fn);
    void* getNativeIndexBufferStructForHandle(IndexBufferHandle ibh);

    typedef void* (*GetIndexBufferForHandleFn)(IndexBufferHandle ibh);
    void registerGetIndexBufferForHandle(GetIndexBufferForHandleFn fn);
    void* getIndexBufferForHandle(IndexBufferHandle ibh);

    typedef void* (*GetTextureForHandleFn)(TextureHandle ibh);
    void registerGetTextureForHandle(GetTextureForHandleFn fn);
    void* getTextureForHandle(TextureHandle ibh);

}

#endif //RENDERER_H
