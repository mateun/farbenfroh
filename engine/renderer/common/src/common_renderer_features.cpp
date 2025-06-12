//
// Created by mgrus on 25.04.2025.
//
#include <renderer.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#define STB_IMAGE_IMPLEMENTATION
#include <fstream>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>
#include "stb_image.h"
#include <unordered_map>
#include <nljson.h>
#include <skeleton.h>
#include <Joint.h>

#include "util.h"
// #include "../../../../v2025/extlibs/glew-2.2.0/include/GL/glew.h"
// #include <GL/glew.h>

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
    static CreateVertexBufferFn g_createVertexBufferFn = nullptr;
    static UpdateIndexBufferFn g_updateIndexBuffer = nullptr;
    static VertexBufferBuilderFn g_vertexBufferBuilder = nullptr;
    static DrawTextIntoQuadFn g_drawTextIntoQuad = nullptr;
    static GetVertexBufferForHandleFn g_getVertexBufferForHandleFn = nullptr;
    static GetIndexBufferForHandleFn g_getIndexBufferForHandleFn = nullptr;
    static GetNativeIndexBufferStructForHandleFn g_getNativeIndexBufferStructForHandle = nullptr;
    static GetTextureForHandleFn g_getTextureForHandleFn = nullptr;
    static CreateMeshFn g_createMesh = nullptr;
    static ImportMeshFn g_importMesh = nullptr;

    void registerCreateTexture(CreateTextureFn fn) {
        g_createTexture = fn;
    }

    void registerVertexBufferBuilder(VertexBufferBuilderFn fn) {
        g_vertexBufferBuilder = fn;
    }
    void registerCreateVertexBuffer(CreateVertexBufferFn fn) {
        g_createVertexBufferFn = fn;
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

    void registerGetNativeIndexBufferStructForHandle(GetNativeIndexBufferStructForHandleFn fn) {
        g_getNativeIndexBufferStructForHandle = fn;
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


    VertexBufferHandle createVertexBuffer(VertexBufferCreateInfo vbd) {
        return g_createVertexBufferFn(vbd);
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

    void* getNativeIndexBufferStructForHandle(IndexBufferHandle ibh) {
        return g_getNativeIndexBufferStructForHandle(ibh);
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

        /**
     * We only accept scenes with 1 mesh for now.
     * @param gltfJson the parsed json of the gltf
     * @return A Pointer to a newly created mesh object.
     */
     Mesh parseGLTF(const std::string& filename) {

        std::ifstream file(filename, std::ios::binary);
        if (!file) {
         throw std::runtime_error("Failed to open GLB file.");
        }

        // Seek to end to get size
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        // Read all data
        std::vector<uint8_t> glbData(fileSize);
        file.read(reinterpret_cast<char*>(glbData.data()), fileSize);

        // -- Parse header
        uint32_t magic = *reinterpret_cast<uint32_t*>(&glbData[0]);
        uint32_t version = *reinterpret_cast<uint32_t*>(&glbData[4]);
        uint32_t length = *reinterpret_cast<uint32_t*>(&glbData[8]);

        assert(magic == 0x46546C67);  // "glTF"
        assert(version == 2);

        // -- First chunk: JSON
        uint32_t jsonChunkLength = *reinterpret_cast<uint32_t*>(&glbData[12]);
        uint32_t jsonChunkType = *reinterpret_cast<uint32_t*>(&glbData[16]);
        assert(jsonChunkType == 0x4E4F534A);  // "JSON"

        std::string jsonText(reinterpret_cast<const char*>(&glbData[20]), jsonChunkLength);

        // Parse JSON
        using json = nlohmann::json;
        json gltf = json::parse(jsonText);

        // -- Second chunk: BIN
        size_t binChunkStart = 20 + jsonChunkLength;
        uint32_t binChunkLength = *reinterpret_cast<uint32_t*>(&glbData[binChunkStart]);
        uint32_t binChunkType = *reinterpret_cast<uint32_t*>(&glbData[binChunkStart + 4]);
        assert(binChunkType == 0x004E4942); // "BIN"

        const uint8_t* binaryBuffer = &glbData[binChunkStart + 8];
        std::vector<uint8_t> dataBinary(binaryBuffer, binaryBuffer + binChunkLength);


        // Armature data for skeletal mesh.
        // If we find a skin within the mesh object.
        // Otherwise we assume a static mesh.
        Skeleton* skeleton = nullptr;
        const auto nodesNode = gltf["/nodes"_json_pointer];
        int skinIndex = -1;
        for (auto n : nodesNode) {

            if (n.is_object()) {
                if (n.contains("skin") && n["skin"].is_number_integer()) {
                    skinIndex = n["skin"];
                    break;
                }
            }
        }

        if (skinIndex > -1) {
            skeleton = new Skeleton();
            auto skinNode = gltf[json::json_pointer("/skins/" + std::to_string(skinIndex))];
            int inverseBindMatricesIndex = skinNode.value("inverseBindMatrices", -1);
            auto inverseBindAccessor = gltf[json::json_pointer("/accessors/" + std::to_string(inverseBindMatricesIndex))];
            auto inverseBindViewIndex = inverseBindAccessor.value("bufferView", -1);
            auto inverseBindBufferView = gltf[json::json_pointer("/bufferViews/" + std::to_string(inverseBindViewIndex))];
            auto inverseBindBufferIndex = inverseBindBufferView.value("buffer", -1);
            auto inverseBindBufferLen = inverseBindBufferView.value("byteLength", -1);
            auto inverseBindBufferByteOffset = inverseBindBufferView.value("byteOffset", -1);

            auto invBindMatricesOffset = dataBinary.data() + inverseBindBufferByteOffset;
            int count = 1;
            // One matrix is 16 values, each 4 values form a column.
            std::vector<glm::mat4> invBindMatrices;
            std::vector<float> vals;
            std::vector<glm::vec4> vecs;
            for ( int i = 0; i < inverseBindBufferLen; i+=4) {
                auto val= (float*)(invBindMatricesOffset + i);
                printf("val: %f\n", *val);
                vals.push_back(*val);
                if (count % 4 == 0) {
                    //printf("col------------\n");
                    vecs.push_back({vals[0], vals[1], vals[2], vals[3]});
                    vals.clear();
                }
                if (count % 16 == 0) {
                    //printf("matrix ----------\n");
                    invBindMatrices.push_back(glm::mat4(vecs[0], vecs[1], vecs[2], vecs[3]));
                    vecs.clear();
                }
                count++;
            }
            // We search for a node called "Armature"
            for (const auto& obj : nodesNode) {
                std::string name = obj.value("name", "");

                if (strContains(name, "Armature")) {
                    if (obj.contains("children") && obj["children"].is_array()) {
                        const auto& children = obj["children"];
                        // Legacy code:
                        // collectJoints(children, skeleton->joints, nodesNode->value->arrayValue, nullptr);

                        // Partial new code
                        // loop over `children` or access it as needed
                        for (const auto& childIndex : children) {
                            // childIndex is typically an integer (node index)
                            int index = childIndex;
                            // use index...
                        }
                    }
                }
            }

            // Now we can associate the joints with the respective inverse bind matrix
            for (int i =0; i < skeleton->joints.size(); i++) {
                auto j = skeleton->joints[i];
                auto invBindMat = invBindMatrices[i];
                j->inverseBindMatrix = invBindMat;
            }

            // Now for the animations
            // Every animation has a number of references to so-called channel samplers.
            // Each sampler describes a specific joint and its translation, scale or rotation.
            // The sampler index points to the samplers array.
            // Each sampler maps a time point to an animation. (input -> output).
            // The indices within the sampler are accessor indices.
            // Reading out the animations:
            // For now just 1 animation.
            auto animationsNode = gltf["/animations"_json_pointer];

                for (auto animNode : animationsNode) {

                    auto samplersNode = animNode["/samplers"_json_pointer];
                    auto animName = animNode.value("name", "");
                    auto channelsNode = animNode.value("channels", json::object());
                    auto accessors = gltf["/accessors"_json_pointer];
                    auto bufferViews =gltf["/bufferViews"_json_pointer];
                    for (auto ch : channelsNode) {
                        int samplerIndex = ch.value("sampler", -1);
                        auto targetNode = ch["/target"_json_pointer];
                        auto targetPath = targetNode.value("path", "");
                        int targetJointIndex = targetNode.value("node", json::object());
                        auto jointNode = nodesNode[targetJointIndex];
                        auto jointName = jointNode.value("name", "");
                        printf("joint channel:%s %s\n", jointName.c_str(), targetPath.c_str());
                        auto samplerNode = samplersNode[samplerIndex];
                        auto samplerInput = samplerNode.value("input", 0);
                        auto samplerOutput = samplerNode.value("output", 0);
                        auto samplerInterpolation = samplerNode.value("interpolation", "");
                        // Now lookup the accessors for input and output
                        auto inputAccessor = accessors[samplerInput];
                        auto outputAccessor = accessors[samplerOutput];
                        auto inputCount = inputAccessor.value("count", -1);
                        auto inputType = inputAccessor.value("type", "");
                        auto outputType = outputAccessor.value("type", "");
                        auto outputCount = outputAccessor.value("count", 0.0);
                        auto inputBufferViewIndex = inputAccessor.value("bufferView", -1);
                        auto outputBufferViewIndex = outputAccessor.value("bufferView", -1);
                        auto inputBufferView = bufferViews[inputBufferViewIndex];
                        auto outputBufferView = bufferViews[outputBufferViewIndex];
                        auto inputBufferOffset = inputBufferView.value("byteOffset", -1);
                        auto inputBufferLength = inputBufferView.value("byteLength", -1);
                        auto outputBufferOffset = outputBufferView.value("byteOffset", 0);
                        auto outputBufferLength = outputBufferView.value("byteLength", 0);
                        auto inputDataPtr = dataBinary.data() + inputBufferOffset;
                        std::vector<float> timeValues;
                        for ( int i = 0; i < inputBufferLength; i+=4) {
                            auto val= (float*)(inputDataPtr + i);
                            timeValues.push_back(*val);
                        }
                        auto outputDataPtr = dataBinary.data() + outputBufferOffset;
                        if (outputType == "VEC3") {
                            std::vector<glm::vec3> outputValues;
                            std::vector<float> fvals;
                            count = 1;
                            for ( int i = 0; i < outputBufferLength; i+=4) {
                                auto val= (float*)(outputDataPtr + i);
                                fvals.push_back(*val);
                                if (count % 3 == 0) {
                                    outputValues.push_back(glm::vec3{fvals[0], fvals[1], fvals[2]});
                                    fvals.clear();
                                }
                                count++;
                            }
                            for (int i=0; i<timeValues.size(); i++) {
                                printf("time val: %f -> %f/%f/%f\n", timeValues[i], outputValues[i].x, outputValues[i].y,
                                       outputValues[i].z);
                            }

                        } else if (outputType == "VEC4") {
                            std::vector<glm::vec4> outputValues;
                            std::vector<float> fvals;
                            count = 1;
                            for ( int i = 0; i < outputBufferLength; i+=4) {
                                auto val= (float*)(outputDataPtr + i);
                                fvals.push_back(*val);
                                if (count % 4 == 0) {
                                    outputValues.push_back(glm::vec4{fvals[0], fvals[1], fvals[2], fvals[3]});

                                    glm::quat quat = {fvals[3], fvals[0], fvals[1], fvals[2]};
                                    glm::mat4 rotMat = glm::toMat4(quat);
                                    printf("rotmat: %f/%f/%f/%f\n",
                                           glm::column(rotMat, 0).x,
                                           glm::column(rotMat, 1).x,
                                           glm::column(rotMat, 2).x,
                                           glm::column(rotMat, 3).x);
                                    printf("rotmat: %f/%f/%f/%f\n",
                                           glm::column(rotMat, 0).y,
                                           glm::column(rotMat, 1).y,
                                           glm::column(rotMat, 2).y,
                                           glm::column(rotMat, 3).y);
                                    printf("rotmat: %f/%f/%f/%f\n",
                                           glm::column(rotMat, 0).z,
                                           glm::column(rotMat, 1).z,
                                           glm::column(rotMat, 2).z,
                                           glm::column(rotMat, 3).z);
                                    printf("rotmat: %f/%f/%f/%f\n",
                                           glm::column(rotMat, 0).w,
                                           glm::column(rotMat, 1).w,
                                           glm::column(rotMat, 2).w,
                                           glm::column(rotMat, 3).w);
                                    fvals.clear();
                                }
                                count++;
                            }
                            for (int i=0; i<timeValues.size(); i++) {
                                printf("time val: %f -> %f/%f/%f/%f\n", timeValues[i], outputValues[i].x, outputValues[i].y,
                                       outputValues[i].z, outputValues[i].w);
                            }
                        } else if (outputType == "SCALAR") {
                            // TODO scalar
                        }
                    }
                }

        }

        // Mesh data
        auto meshesNode = gltf["/meshes"_json_pointer];
        int mesh_index = 0;
        for (auto m : meshesNode) {
            auto prim_node = m["primitives"];
            for (auto p : prim_node) {
                auto att = p["attributes"];
                std::cout << att["POSITION"].dump(2) << std::endl;
            }
        }
        auto primitivesNode = gltf["/meshes/0/primitives/0"_json_pointer];
        auto attrObj = primitivesNode["attributes"];

        int posIndex = attrObj["POSITION"].get<int>();
        auto uvIndex = attrObj["TEXCOORD_0"].get<int>();
        auto normalIndex =attrObj["NORMAL"].get<int>();
        auto indicesIndex = primitivesNode["indices"].get<int>();

        auto posAccessor = gltf[json::json_pointer("/accessors/" + std::to_string(posIndex))];
        auto uvAccessor =gltf[json::json_pointer("/accessors/" + std::to_string(uvIndex))];
        auto normalAccessor = gltf[json::json_pointer("/accessors/" + std::to_string(normalIndex))];
        auto indicesAccessor = gltf[json::json_pointer("/accessors/" + std::to_string(indicesIndex))];

        auto posBufferViewIndex = posAccessor["bufferView"].get<int>();
        auto posBufferView = gltf[json::json_pointer("/bufferViews/" + std::to_string(posBufferViewIndex))];
        auto posBufferIndex = posBufferView["buffer"].get<int>();
        auto posBufferLen = posBufferView["byteLength"].get<float>();
        auto posBufferByteOffset = posBufferView["byteOffset"].get<int>();
        auto posGlTargetType = posBufferView["target"].get<int>();

        auto uvBufferViewIndex = uvAccessor["bufferView"].get<int>();
        auto uvBufferView =gltf[json::json_pointer("/bufferViews/" + std::to_string(uvBufferViewIndex))];
        auto uvBufferIndex = uvBufferView["buffer"].get<int>();
        auto uvBufferLen = uvBufferView["byteLength"].get<float>();
        auto uvBufferByteOffset = uvBufferView["byteOffset"].get<int>();
        auto uvGlTargetType = uvBufferView["target"].get<int>();

        auto normalBufferViewIndex = normalAccessor["bufferView"].get<int>();
        auto normalBufferView = gltf[json::json_pointer("/bufferViews/" + std::to_string(normalBufferViewIndex))];
        auto normalBufferIndex = normalBufferView["buffer"].get<int>();
        auto normalBufferLen = normalBufferView["byteLength"].get<int>();
        auto normalBufferByteOffset = normalBufferView["byteOffset"].get<int>();
        auto normalGlTargetType = normalBufferView["target"].get<int>();

        auto indicesBufferViewIndex = indicesAccessor["bufferView"].get<int>();
        auto indicesBufferView = gltf[json::json_pointer("/bufferViews/" + std::to_string(indicesBufferViewIndex))];
        auto indicesBufferIndex = indicesBufferView["buffer"].get<int>();
        auto indicesBufferLen = indicesBufferView["byteLength"].get<int>();
        auto indicesBufferByteOffset = indicesBufferView["byteOffset"].get<int>();
        auto indicesGlTargetType = indicesBufferView["target"].get<int>();
        auto indexCount = indicesAccessor["count"].get<int>();
        auto indexComponentType = indicesAccessor["componentType"].get<int>();

        VertexBufferCreateInfo ci;
        const uint8_t* posBase = dataBinary.data() + posBufferByteOffset;
        const float* posData = reinterpret_cast<const float*>(posBase);
        size_t totalFloats = posAccessor["count"].get<int>() * 3;

        std::vector<glm::vec3> positions;
        for (int i = 0; i< totalFloats; i +=3) {
            positions.push_back({posData[i], posData[i+1], posData[i+2]});
        }

        const uint8_t* uvBase = dataBinary.data() + uvBufferByteOffset;
        const float* uvData = reinterpret_cast<const float*>(uvBase);
        size_t totalUvs = uvAccessor["count"].get<int>() * 2;
        std::vector<glm::vec2> uvs;
        for (int i = 0; i< totalUvs; i += 2) {
            uvs.push_back({uvData[i], uvData[i+1]});
        }

        for (int i = 0; i < positions.size(); i++) {
            ci.data.push_back(positions[i].x);
            ci.data.push_back(positions[i].y *-1 );     // TODO: We should only do this *-1 in vulkan, which we are not aware of here
            ci.data.push_back(positions[i].z);
            ci.data.push_back(uvs[i].x);
            ci.data.push_back(uvs[i].y);                 // TODO: only in vulkan, but we are not aware here.

        }

        ci.stride = 0;
        auto vbo = createVertexBuffer(ci);

        const uint8_t* indices = dataBinary.data() + indicesBufferByteOffset;

        IndexBufferDesc ibd;
        ibd.size_in_bytes = indicesBufferLen;
        ibd.format = indexComponentType;
        ibd.data = indices;
        IndexBufferHandle ibo = createIndexBuffer(ibd);

        VertexAttribute posAttribute;
        posAttribute.location = 0;
        posAttribute.stride = 0;
        posAttribute.components = 3;
        posAttribute.format = VertexAttributeFormat::Float3;
        posAttribute.offset = 0;
        posAttribute.semantic = VertexAttributeSemantic::Position;
        auto mesh = createMesh(vbo, ibo, {posAttribute}, indexCount);
        return mesh;

    }




}
