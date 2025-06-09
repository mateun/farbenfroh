//
// Created by mgrus on 25.04.2025.
//
#include <renderer.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <unordered_map>

#include "../../../../v2025/extlibs/glew-2.2.0/include/GL/glew.h"

struct Font {
    uint32_t id;
    renderer::TextureHandle fontAtlas;
    float maxDescent = std::numeric_limits<float>::max();
    float lineHeight = std::numeric_limits<float>::min();
    float baseLine = 0.0f;
    std::vector<stbtt_bakedchar> bakedChars;

};

static uint32_t nextHandleId = 1; // start at 1 to reserve 0 as "invalid"
static std::unordered_map<uint32_t, Font> fontMap;

namespace renderer {
    // We have function-pointers to all the concrete implementation which
    // we need here from common, but can not implement ourselves.
    // With this method, we do not need to statically link ourselves (as common dll)
    // to any concrete renderer backend, but can still call the correct impl. at runtime.
    // We provide registration functions which the concrete impls then need to call to
    // register themselves as "providers" of the respective functionality.
    static CreateTextureFn g_createTexture = nullptr;
    static CreateIndexBufferFn g_createIndexBuffer = nullptr;
    static UpdateIndexBufferFn g_updateIndexBuffer = nullptr;
    static VertexBufferBuilderFn g_vertexBufferBuilder = nullptr;
    static DrawTextIntoQuadFn g_drawTextIntoQuad = nullptr;
    static GetVertexBufferForHandleFn g_getVertexBufferForHandleFn = nullptr;
    static GetIndexBufferForHandleFn g_getIndexBufferForHandleFn = nullptr;
    static GetTextureForHandleFn g_getTextureForHandleFn = nullptr;
    static CreateMeshFn g_createMesh = nullptr;
    static ImportMeshFn g_importMesh = nullptr;

    void registerCreateTexture(CreateTextureFn fn) {
        g_createTexture = fn;
    }

    void registerVertexBufferBuilder(VertexBufferBuilderFn fn) {
        g_vertexBufferBuilder = fn;
    }

    void registerCreateIndexBuffer(CreateIndexBufferFn fn) {
        g_createIndexBuffer = fn;
    }

    void registerDrawTextIntoQuad(DrawTextIntoQuadFn fn) {
        g_drawTextIntoQuad = fn;
    }

    void registerGetVertexBufferForHandle(GetVertexBufferForHandleFn fn) {
        g_getVertexBufferForHandleFn = fn;
    }

    void registerGetIndexBufferForHandle(GetIndexBufferForHandleFn fn) {
        g_getIndexBufferForHandleFn = fn;
    }

    void registerGetTextureForHandle(GetTextureForHandleFn fn) {
        g_getTextureForHandleFn = fn;
    }

    void registerUpdateIndexBuffer(UpdateIndexBufferFn fn) {
        g_updateIndexBuffer = fn;
    }

    void registerCreateMesh(CreateMeshFn fn) {
        g_createMesh = fn;
    }

    void registerImportMesh(ImportMeshFn fn) {
        g_importMesh = fn;
    }

    TextureHandle createTexture(const Image& image, TextureFormat format) {
        if (!g_createTexture) {
            return {};
        }
        return g_createTexture(image, format);
    }

    IndexBufferHandle createIndexBuffer(const IndexBufferDesc& ibd) {
        if (!g_createIndexBuffer) {
            return {};
        }
        return g_createIndexBuffer(ibd);
    }

    void* getVertexBufferForHandle(VertexBufferHandle vbh) {
        return g_getVertexBufferForHandleFn(vbh);
    }

    void* getIndexBufferForHandle(IndexBufferHandle ibh) {
        return g_getIndexBufferForHandleFn(ibh);
    }

    void* getTextureForHandle(TextureHandle th) {
        return g_getTextureForHandleFn(th);
    }

    std::unique_ptr<VertexBufferBuilder> vertexBufferBuilder() {
        if (!g_vertexBufferBuilder) {
            return {};
        }
        return g_vertexBufferBuilder();
    }

    void updateIndexBuffer(IndexBufferHandle iboHandle, std::vector<uint32_t> data) {
        if (!g_updateIndexBuffer) {
            return;
        }
        g_updateIndexBuffer(iboHandle, data);
    }

    Mesh importMesh(const std::string& filename) {
        if (!g_importMesh) {
            return {};
        }
        return g_importMesh(filename);
    };

    Mesh createMesh(VertexBufferHandle vbo, IndexBufferHandle ibo, const std::vector<VertexAttribute> & attributes, size_t index_count) {
        if (!g_createMesh) {
            return {};
        }
        return g_createMesh(vbo, ibo, attributes, index_count);
    }

    FontHandle createFontFromFile(const std::string &pathToTTF, float fontSize) {
        // Read font file
        FILE *fp = fopen(pathToTTF.c_str(), "rb");
        if (!fp) {
            fprintf(stderr, "Failed to open TTF file.\n");
            // TODO
            //throw std::runtime_error("Failed to open TTF file.");
        }
        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        unsigned char *ttf_buffer = new unsigned char[size];
        fread(ttf_buffer, 1, size, fp);
        fclose(fp);

        Image atlasImage = {
            .pixels = new uint8_t[512 * 512],
            .width = 512,
            .height = 512,
            .channels = 1,

        };

        // Retrieve font measurements
        stbtt_fontinfo info;
        stbtt_InitFont(&info, ttf_buffer, 0);

        Font font;

        int ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
        float scale = stbtt_ScaleForPixelHeight(&info, fontSize);
        auto scaled_ascent_  = ascent  * scale;  // typically a positive number
        auto scaled_descent_ = descent * scale;  // typically negative
        auto scaled_line_gap_ = lineGap * scale;

        font.baseLine = scaled_ascent_;
        font.lineHeight = (scaled_ascent_ - scaled_descent_) + scaled_line_gap_;
        font.bakedChars.resize(96);
        int result = stbtt_BakeFontBitmap(ttf_buffer, 0, fontSize,
                                      atlasImage.pixels, atlasImage.width, atlasImage.height,
                                      32, 96, font.bakedChars.data());

        if (result <= 0) {
            fprintf(stderr, "Failed to bake font bitmap.\n");
            delete[] ttf_buffer;
            // TODO error case?!
        }

        auto atlasTexture = createTexture(atlasImage, TextureFormat::R8);
        font.fontAtlas = atlasTexture;

        FontHandle fontHandle = {nextHandleId};
        fontHandle.atlasTexture = atlasTexture;
        fontMap[nextHandleId] = font;
        nextHandleId++;
        return fontHandle;

    }

    Image createImageFromFile(const std::string &filename) {
        Image image;
        auto pixels = stbi_load(filename.c_str(), &image.width, &image.height,
                &image.channels,
                4);
        image.pixels = pixels;
        return image;
    }

    void drawTextIntoQuadGeometry(FontHandle fontHandle, const std::string& text,
        std::vector<glm::vec3>& outPositions, std::vector<glm::vec2>& outUVs, std::vector<uint32_t>& outIndices) {

        auto font = fontMap[fontHandle.id];

        float penX = 0, penY = 0;
        float minX =  std::numeric_limits<float>::max();
        float maxX = -std::numeric_limits<float>::max();
        float minY =  std::numeric_limits<float>::max();
        float maxY = -std::numeric_limits<float>::max();
        float baseline = font.baseLine;
        int charCounter = 0;
        for (auto c : text) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font.bakedChars.data(), 512, 512, c - 32, &penX, &penY, &q, 0);

            float pixel_aligned_x0 = std::floor(q.x0 + 0.5f);
            float pixel_aligned_y0 = std::floor(q.y0 + 0.5f);
            float pixel_aligned_x1 = std::floor(q.x1 + 0.5f);
            float pixel_aligned_y1 = std::floor(q.y1 + 0.5f);

            q.x0 = pixel_aligned_x0;
            q.y0 = pixel_aligned_y0;
            q.x1 = pixel_aligned_x1;
            q.y1 = pixel_aligned_y1;

            outPositions.push_back(glm::vec3(q.x0, q.y0, 0));
            outPositions.push_back(glm::vec3(q.x1, q.y0, 0));
            outPositions.push_back(glm::vec3(q.x1, q.y1, 0));
            outPositions.push_back(glm::vec3(q.x0, q.y1, 0));


            outUVs.push_back({q.s0, q.t0});
            outUVs.push_back({q.s1, q.t0});
            outUVs.push_back({q.s1, q.t1});
            outUVs.push_back({q.s0, q.t1});

            // Flip vertical uv coordinates
            // uvs.push_back({q.s0, q.t1});
            // uvs.push_back({q.s1, q.t1});
            // uvs.push_back({q.s1, q.t0});
            // uvs.push_back({q.s0, q.t0});


            int offset = charCounter * 4;
            outIndices.push_back(2 + offset);outIndices.push_back(1 + offset);outIndices.push_back(0 + offset);
            outIndices.push_back(2 + offset);outIndices.push_back(0 + offset);outIndices.push_back(3 + offset);

            // Flipped
            // indices.push_back(0 + offset);indices.push_back(1 + offset);indices.push_back(2 + offset);
            // indices.push_back(3 + offset);indices.push_back(0 + offset);indices.push_back(2 + offset);
            charCounter++;

           // Track min/max for bounding box
            minX = std::min(minX, q.x0);
            maxX = std::max(maxX, q.x1);

            if (c == 32) continue; // ignore space for Y, as this is always zero and messes things up.
            minY = std::min(minY, q.y0); // lowest part (descenders)
            minY = std::min(minY, q.y1);

            maxY = std::max(maxY, q.y0); // highest part (ascenders)
            maxY = std::max(maxY, q.y1);
        }

    }

    BoundingBox measureText(FontHandle fontHandle, const std::string& text) {
        auto font = fontMap[fontHandle.id];
        float penX = 0, penY = 0;
        float minX =  std::numeric_limits<float>::max();
        float maxX = -std::numeric_limits<float>::max();
        float minY =  std::numeric_limits<float>::max();
        float maxY = -std::numeric_limits<float>::max();
        for (auto c : text) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font.bakedChars.data(), 512, 512, c - 32, &penX, &penY, &q, 0);

            float pixel_aligned_x0 = std::floor(q.x0 + 0.5f);
            float pixel_aligned_y0 = std::floor(q.y0 + 0.5f);
            float pixel_aligned_x1 = std::floor(q.x1 + 0.5f);
            float pixel_aligned_y1 = std::floor(q.y1 + 0.5f);

            q.x0 = pixel_aligned_x0;
            q.y0 = pixel_aligned_y0;
            q.x1 = pixel_aligned_x1;
            q.y1 = pixel_aligned_y1;

           // Track min/max for bounding box
            minX = std::min(minX, q.x0);
            maxX = std::max(maxX, q.x1);

            if (c == 32) continue; // ignore space for Y, as this is always zero and messes things up.
            minY = std::min(minY, q.y0); // lowest part (descenders)
            minY = std::min(minY, q.y1);

            maxY = std::max(maxY, q.y0); // highest part (ascenders)
            maxY = std::max(maxY, q.y1);
        }
        return BoundingBox(minX, minY, maxX, maxY);
    }

    Mesh drawTextIntoQuad(FontHandle fontHandle, const std::string& text) {
        if (!g_drawTextIntoQuad) {
            return {};
        }
        return g_drawTextIntoQuad(fontHandle, text);

    }

    void updateText(Mesh& mesh, FontHandle fontHandle, const std::string& newText) {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<uint32_t> indices;
        drawTextIntoQuadGeometry(fontHandle, newText, positions, uvs, indices);
        g_vertexBufferBuilder()->attributeVec3(VertexAttributeSemantic::Position, positions)
            .attributeVec2(VertexAttributeSemantic::UV0, uvs).update(mesh.vertex_buffer);
        updateIndexBuffer(mesh.index_buffer, indices);
        mesh.index_count = indices.size();

    }


    GLSLVertexShaderBuilder & GLSLVertexShaderBuilder::position(uint8_t slot) {
        hasPosition = true;
        positionSlot = slot;
        return *this;
    }

    GLSLVertexShaderBuilder & GLSLVertexShaderBuilder::normal(uint8_t slot) {
        hasNormal = true;
        normalSlot = slot;
        return *this;
    }

    GLSLVertexShaderBuilder & GLSLVertexShaderBuilder::uv(uint8_t slot) {
        hasUV = true;
        uvSlot = slot;
        return *this;
    }

    VertexShaderBuilder & GLSLVertexShaderBuilder::mvp() {
        hasMVPUniforms = true;
        return *this;
    }

    VertexShaderBuilder & GLSLVertexShaderBuilder::worldMatrix() {
        hasWorldMatrixUniform = true;
        return *this;
    }

    VertexShaderBuilder & GLSLVertexShaderBuilder::projectionMatrix() {
        hasWorldMatrixUniform = true;
        return *this;
    }

    VertexShaderBuilder & GLSLVertexShaderBuilder::viewMatrix() {
        hasWorldMatrixUniform = true;
        return *this;
    }

    GLSLVertexShaderBuilder & GLSLVertexShaderBuilder::uniform(renderer::CustomUniform customUniform) {
        custom_uniforms.push_back(customUniform);
        return *this;
    }

    VertexShaderBuilder & GLSLVertexShaderBuilder::uniformBufferObject(CustomUniformBufferObject ubo) {
        custom_uniform_buffer_objects.push_back(ubo);
        return *this;
    }

    std::unique_ptr<VertexShaderBuilder>  vertexShaderBuilder() {
        return std::make_unique<GLSLVertexShaderBuilder>();
    }

    std::unique_ptr<FragmentShaderBuilder> fragmentShaderBuilder() {
        return std::make_unique<GLSLFramgentShaderBuilder>();
    }


    std::string GLSLVertexShaderBuilder::build() const {
        std::string src = "#version 450 core\n";

        // Declare vertex attributes:
        if (hasPosition)
            src += "layout(location = " + std::to_string(positionSlot) + ") in vec3 aPosition;\n";
        if (hasNormal)
            src += "layout(location = " + std::to_string(normalSlot) + ") in vec3 aNormal;\n";
        if (hasUV)
            src += "layout(location = " + std::to_string(uvSlot) +") in vec2 aUV;\n";

        // Declare uniform buffer objects:
        for (auto ubo : custom_uniform_buffer_objects) {
            src += "\n";
            src += "layout(set=" + std::to_string(ubo.set);
            src += ", binding=" + std::to_string(ubo.binding);
            src += ") uniform " + ubo.struct_name + " {\n";
            for (auto un : ubo.nested_uniforms) {
                src += "\t" + un.declaration    + ";\n";
            }
            src += "} " + ubo.name + ";\n\n";
        }

        // Declare uniforms:
        if (hasMVPUniforms) {
            src += "layout(set =0, binding = 0) uniform MVPBlock { \n mat4 mvpMatrix;\n } mvp_block; \n";
        }

        if (hasWorldMatrixUniform) {
            src += "uniform mat4 world_mat;\n";
        }

        if (hasProjectionMatrixUniform) {
            src += "uniform mat4 proj_mat;\n";
        }

        if (hasViewMatrixUniform) {
            src += "uniform mat4 view_mat;\n";
        }

        for (auto cu : custom_uniforms) {
            src += cu.declaration + "\n";
        }

        std::string mvpPart = "";
        if (hasMVPUniforms) {
            mvpPart += "mvp_block.mvpMatrix *";
        }
        if (hasWorldMatrixUniform) {
            mvpPart += "world_mat *";
        }

        if (hasProjectionMatrixUniform) {
            mvpPart += "proj_mat *";
        }

        if (hasViewMatrixUniform) {
            mvpPart += "view_mat *";
        }

        // Declare outputs:
        if (hasUV) {
            src += "layout(location = 0) out vec2 fs_uvs;\n";
        }

        src += "void main() {\n";

        // Calculate clip-space position:
        if (hasPosition) {
            src += "\tgl_Position = " + mvpPart + " vec4(aPosition, 1.0);\n";
        }
        else {
            src += "    gl_Position = vec4(0.0);\n";
        }

        // Assign other outputs:
        if (hasUV) {
            src += "    fs_uvs = aUV;\n";
            // TODO handle uv flipping
            //src += "    fs_uvs.y = 1 - fs_uvs.y;\n";
        }

        for (auto cu : custom_uniforms) {
            src += cu.custom_code + "\n";
        }

        for (auto ubo : custom_uniform_buffer_objects) {
            src += "\n";
            for (auto cu : ubo.nested_uniforms) {
                src += cu.custom_code + "\n";
            }
        }

        src += "}\n";

        return src;
    }

    renderer::FragmentShaderBuilder & GLSLFramgentShaderBuilder::color() {
        useColor = true;
        return *this;
    }

    renderer::FragmentShaderBuilder & GLSLFramgentShaderBuilder::diffuseTexture(uint8_t textureUnit, bool flipUVs) {
        useDiffuseTexture = true;
        diffuseTextureUnit = textureUnit;
        this->flipUVs = flipUVs;
        return *this;
    }

    renderer::FragmentShaderBuilder & GLSLFramgentShaderBuilder::textRender() {
        useTextRender = true;
        return *this;
    }

    std::string GLSLFramgentShaderBuilder::build() const {
        std::string src = "#version 460 core\n";

        // Declare inputs:
        if (useColor) {
            src += "uniform vec4 color = vec4(1, 1, 1, 1);\n";
        }
        if (useDiffuseTexture || useTextRender) {


            src += "layout(binding = " + std::to_string(diffuseTextureUnit) + ") uniform sampler2D diffuseTexture;\n\n";
            src += "layout (location = 0) in vec2 fs_uvs;\n";
        }



        src += "out vec4 final_color;\n";
        src += "void main() {\n";

        if (useColor) {
            src += "    final_color = color;\n";
        }
        else if (useDiffuseTexture ) {
            // Move the uv into a local variable so we can modify it:
            src += "vec2 uv = fs_uvs;\n";

            // Account for uv-flipping:
            if (flipUVs) {
                src += "uv.y = 1.0- uv.y;\n";
            }

            src += "    final_color = texture(diffuseTexture, uv);\n";
        }
        else {
            src += "    final_color = vec4(1, 0,1, 1);\n";
        }

        if (useTextRender) {
            src += "   float r =  texture(diffuseTexture, uv).r;\n";
            // TODO allow setting of textcolor
            src += "   final_color = vec4(1, 1 , 1, r);\n";

        }




        src += "}\n";
        return src;

    }



}
