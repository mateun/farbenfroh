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

#include "skeletal_animation.h"
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

struct ChannelData {
    int samplerIndex;
    nlohmann::json targetNode;
    std::string targetPath;
    int targetJointIndex;
    nlohmann::json jointNode;
    std::string jointName;
    nlohmann::json samplerNode;
    int samplerInput;
    int samplerOutput;
    std::string samplerInterpolation;
    nlohmann::json inputAccessor;
    nlohmann::json outputAccessor;
    int inputCount;
    std::string inputType;
    std::string outputType;
    int outputCount;
    int inputBufferViewIndex;
    int outputBufferViewIndex;
    nlohmann::json inputBufferView;
    nlohmann::json outputBufferView;
    int inputBufferOffset;
    int inputBufferLength;
    int outputBufferOffset;
    int outputBufferLength;
    uint8_t* inputDataPtr;

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
    static GetVertexBufferMemoryForHandleFn g_getVertexBufferMemoryForHandleFn = nullptr;
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

    void registerGetVertexBufferMemoryForHandle(GetVertexBufferMemoryForHandleFn fn) {
        g_getVertexBufferMemoryForHandleFn = fn;
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

    void* getVertexBufferMemoryForHandle(VertexBufferHandle vbh) {
        if (!g_getVertexBufferMemoryForHandleFn) {
            return nullptr;
        }
        return g_getVertexBufferMemoryForHandleFn(vbh);
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
            // outIndices.push_back(2 + offset);outIndices.push_back(1 + offset);outIndices.push_back(0 + offset);
            // outIndices.push_back(2 + offset);outIndices.push_back(0 + offset);outIndices.push_back(3 + offset);

            // Flipped
             outIndices.push_back(0 + offset);outIndices.push_back(1 + offset);outIndices.push_back(2 + offset);
             outIndices.push_back(3 + offset);outIndices.push_back(0 + offset);outIndices.push_back(2 + offset);
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


    /*
void collectJoints(JsonArray* armatureChildren, std::vector<Joint*>& targetVector, const JsonArray* nodes, Joint* parent) {
    for (auto c : armatureChildren->elements) {
        auto childIndex = c->value->floatValue;
        auto childNode = nodes->elements[childIndex]->value->objectValue;

        // Check if this node is the mesh.
        // If yes, skip it, we only collect joints.
        auto mesh = findByMemberName(childNode, "mesh");
        if (mesh) {
            continue;
        }
        auto name = findByMemberName(childNode, "name")->stringValue;
        Joint* joint = new Joint();
        joint->name = name;
        targetVector.push_back(joint);
        if (parent) {
            parent->children.push_back(joint);
        }
        auto translationArray = findByMemberName(childNode, "translation");
        if (translationArray && !translationArray->arrayValue->elements.empty()) {
            auto translationVertices = translationArray->arrayValue->elements;
            joint->translation = glm::vec3{translationVertices[0]->value->floatValue,
                                           translationVertices[1]->value->floatValue,
                                           translationVertices[2]->value->floatValue};
            joint->modelTranslation = glm::translate(glm::mat4(1), joint->translation);
        }

        auto rotationArray = findByMemberName(childNode, "rotation");
        if (rotationArray && !rotationArray->arrayValue->elements.empty()) {
            auto rotationValues = rotationArray->arrayValue->elements;
            joint->rotation = glm::quat(rotationValues[3]->value->floatValue,
                                        rotationValues[0]->value->floatValue,
                                        rotationValues[1]->value->floatValue,
                                        rotationValues[2]->value->floatValue);


            joint->modelRotation = glm::toMat4(joint->rotation);

        }

        joint->currentPoseLocalTransform = joint->modelTranslation * joint->modelRotation;
        joint->currentPoseGlobalTransform = parent ? (parent->currentPoseGlobalTransform * joint->currentPoseLocalTransform) : joint->currentPoseLocalTransform;

        auto children = findByMemberName(childNode, "children");
        if (children) {
            collectJoints(children->arrayValue, targetVector, nodes, joint);
        }
    }
}
*/


    ChannelData extractChannelData(nlohmann::basic_json<> ch, nlohmann::basic_json<> nodesNode, nlohmann::json samplersNode,
        nlohmann::json accessors, nlohmann::json bufferViews, std::vector<uint8_t>& dataBinary) {
        using namespace  nlohmann;
        ChannelData cd;

        cd.samplerIndex = ch.value("sampler", -1);
        cd.targetNode = ch["/target"_json_pointer];
        cd.targetPath = cd.targetNode.value("path", "");
        cd.targetJointIndex = cd.targetNode.value("node", json::object());
        cd.jointNode = nodesNode[cd.targetJointIndex];
        cd.jointName = cd.jointNode.value("name", "");
        cd.samplerNode = samplersNode[cd.samplerIndex];
        cd.samplerInput = cd.samplerNode.value("input", 0);
        cd.samplerOutput = cd.samplerNode.value("output", 0);
        cd.samplerInterpolation = cd.samplerNode.value("interpolation", "");
        cd.inputAccessor = accessors[cd.samplerInput];
        cd.outputAccessor = accessors[cd.samplerOutput];
        cd.inputCount = cd.inputAccessor.value("count", -1);
        cd.inputType = cd.inputAccessor.value("type", "");
        cd.outputType = cd.outputAccessor.value("type", "");
        cd.outputCount = cd.outputAccessor.value("count", 0.0);
        cd.inputBufferViewIndex = cd.inputAccessor.value("bufferView", -1);
        cd.outputBufferViewIndex = cd.outputAccessor.value("bufferView", -1);
        cd.inputBufferView = bufferViews[cd.inputBufferViewIndex];
        cd.outputBufferView = bufferViews[cd.outputBufferViewIndex];
        cd.inputBufferOffset = cd.inputBufferView.value("byteOffset", -1);
        cd.inputBufferLength = cd.inputBufferView.value("byteLength", -1);
        cd.outputBufferOffset = cd.outputBufferView.value("byteOffset", 0);
        cd.outputBufferLength = cd.outputBufferView.value("byteLength", 0);
        cd.inputDataPtr = dataBinary.data() + cd.inputBufferOffset;

        return cd;
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

            // Looking for the first skin we find.
            // So, currently, only one skin supported.
            if (n.is_object()) {
                if (n.contains("skin") && n["skin"].is_number_integer()) {
                    skinIndex = n["skin"];
                    break;
                }
            }
        }

        std::vector<SkeletalAnimation*> animations;
        if (skinIndex > -1) {
            std::map<std::string, Joint*> jointMap;
            skeleton = new Skeleton();
            auto skinNode = gltf[json::json_pointer("/skins/" + std::to_string(skinIndex))];
            int inverseBindMatricesIndex = skinNode.value("inverseBindMatrices", -1);
            auto skin_joints_index_array = skinNode["joints"];
            std::vector<int> joint_indices;
            for (auto ind : skin_joints_index_array) {
                joint_indices.push_back(ind);
            }
            auto inverseBindAccessor = gltf[json::json_pointer("/accessors/" + std::to_string(inverseBindMatricesIndex))];
            auto inverseBindViewIndex = inverseBindAccessor.value("bufferView", -1);
            auto inverseBindBufferView = gltf[json::json_pointer("/bufferViews/" + std::to_string(inverseBindViewIndex))];
            auto inverseBindBufferIndex = inverseBindBufferView.value("buffer", -1);
            auto inverseBindBufferLen = inverseBindBufferView.value("byteLength", -1);
            auto inverseBindBufferByteOffset = inverseBindBufferView.value("byteOffset", -1);

            auto invBindMatricesOffset = dataBinary.data() + inverseBindBufferByteOffset;

            // One matrix is 16 values, each 4 values form a column.
            // We must also store the index of the joint for this bind matrix to find them back together later.
            std::map<int, glm::mat4> invBindMatrices;
            std::vector<float> vals;
            std::vector<glm::vec4> vecs;





            std::map<int, Joint*> node_index_joint_map;
            // First pass: create joint objects and set inverse bind matrix for each joint:
            for (int  j_count =0 ; j_count < joint_indices.size(); j_count++) {
                auto joint_json = nodesNode[joint_indices[j_count]];
                Joint* joint = new Joint();
                skeleton->joints.push_back(joint);
                node_index_joint_map[joint_indices[j_count]] = joint;
                joint->name = joint_json["name"];


                // We need 4 floats per column,
                // and 4 columsn for a matrix:
                std::vector<glm::vec4> col_vecs;
                for (int col = 0; col < 4; col++) {
                    glm::vec4 col_val;
                    for (int val_index = 0;  val_index < 4; val_index ++ ) {
                        float inv_bind_val = 0;
                        int offset = val_index *4  + col * 16 + (j_count * 16 * 4);
                        std::memcpy(&inv_bind_val, invBindMatricesOffset + offset, 4);
                        col_val[val_index] = inv_bind_val;

                    }
                    col_vecs.push_back(col_val);
                }
                joint->inverseBindMatrix = glm::mat4(col_vecs[0], col_vecs[1], col_vecs[2], col_vecs[3]);
            }

            // Second pass: parent-child relationship
            for (int  j_count =0 ; j_count < joint_indices.size(); j_count++) {
                auto joint_index = joint_indices[j_count];
                auto joint_json = nodesNode[joint_index];
                Joint* current_parent_joint = nullptr;
                if (joint_json.contains("children")) {
                    const auto& children = joint_json["children"];
                    for (auto c : children) {
                        auto childIndex = c.get<int>();
                        auto childNode = nodesNode[childIndex];
                        // if (childNode.contains("mesh")) {
                        //     continue;
                        // }

                        auto child_name = childNode["name"].get<std::string>();
                        if (jointMap.find(child_name) == jointMap.end()) {
                            auto joint = new Joint();
                            joint->name = child_name;
                            jointMap[child_name] = joint;
                            skeleton->joints.push_back(joint);
                            joint->parent = current_parent_joint;
                            if (joint->parent) {
                                joint->parent->children.push_back(joint);
                            }
                        } else {
                            auto existing_child = jointMap[child_name];
                            existing_child->parent = current_parent_joint;
                            existing_child->parent->children.push_back(existing_child);
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

            //std::reverse(skeleton->joints.begin(), skeleton->joints.end());

            // Now for the animations
            // Every animation has a number of references to so-called channel samplers.
            // Each sampler describes a specific joint and its translation, scale or rotation.
            // The sampler index points to the samplers array.
            // Each sampler maps a time point to an animation. (input -> output).
            // The indices within the sampler are accessor indices.
            // Reading out the animations:
            // We collect information about the animation

            auto animationsNode = gltf["/animations"_json_pointer];

                for (auto animNode : animationsNode) {

                    auto animation = new SkeletalAnimation();
                    animations.push_back(animation);
                    auto samplersNode = animNode["/samplers"_json_pointer];
                    auto animName = animNode.value("name", "");
                    animation->name = animName;
                    auto channelsNode = animNode.value("channels", json::object());
                    auto accessors = gltf["/accessors"_json_pointer];
                    auto bufferViews =gltf["/bufferViews"_json_pointer];
                    for (auto ch : channelsNode) {
                        auto channelData = extractChannelData(ch, nodesNode, samplersNode, accessors, bufferViews, dataBinary);

                        float duration = std::numeric_limits<float>::min();
                        std::vector<float> timeValues;
                        for (int keyFrame  = 0; keyFrame < channelData.inputCount; keyFrame++) {
                                // We start a new KeyFrameChannel here when handling the time values:
                                auto kfc = new KeyFrameChannel();
                                float inputTimeValue;
                                std::memcpy(&inputTimeValue, channelData.inputDataPtr + (keyFrame * 4), 4);
                                timeValues.push_back(inputTimeValue);
                                if (inputTimeValue > duration) {
                                    duration = inputTimeValue;
                                }
                                kfc->time = inputTimeValue;
                                animation->keyFrameChannels.push_back(kfc);
                                kfc->joint_name = channelData.jointName;
                                kfc->type = [kfc, animation](std::string type) {
                                   if (type == "rotation") {
                                       animation->keyFrameChannels_rotation.push_back(kfc);
                                       return ChannelType::rotation;
                                   }
                                    if (type == "translation") {
                                        animation->keyFrameChannels_translation.push_back(kfc);
                                        return ChannelType::translation;
                                    }
                                    return ChannelType::scale;
                                }(channelData.targetPath);

                        }
                        animation->duration = duration;
                        auto outputDataPtr = dataBinary.data() + channelData.outputBufferOffset;
                        int count = 0;
                        if (channelData.outputType == "VEC3") {
                            std::vector<glm::vec3> outputValues;
                            std::vector<float> fvals;
                            count = 1;
                            int output_index = 0;
                            for ( int i = 0; i < channelData.outputCount * 3; i++) {

                                float val;
                                std::memcpy(&val, outputDataPtr + (i * 4), 4);
                                fvals.push_back(val);
                                if (count % 3 == 0) {
                                    outputValues.push_back(glm::vec3{fvals[0], fvals[1], fvals[2]});
                                    fvals.clear();
                                    auto kfc = animation->keyFrameChannels[output_index];
                                    kfc->value_v3 = outputValues[output_index];
                                    output_index++;

                                }
                                count++;
                            }

                        } else if (channelData.outputType == "VEC4") {
                            // This is a quaternion for rotations:
                            std::vector<glm::vec4> outputValues;
                            std::vector<float> fvals;
                            count = 1;
                            int output_index = 0;
                            for ( int i = 0; i < channelData.outputCount * 4; i++) {
                                float val=0;
                                std::memcpy(&val, (outputDataPtr + i *4), 4);;
                                fvals.push_back(val);
                                if (count % 4 == 0) {
                                    outputValues.push_back(glm::vec4{fvals[0], fvals[1], fvals[2], fvals[3]});

                                    glm::quat quat = {fvals[3], fvals[0], fvals[1], fvals[2]};
                                    glm::mat4 rotMat = glm::toMat4(quat);

                                    auto kfc = animation->keyFrameChannels[output_index];
                                    kfc->value_quat = quat;
                                    kfc->value_v4 = outputValues[output_index];
                                    output_index++;
                                    fvals.clear();
                                }
                                count++;
                            }
                        } else if (channelData.outputType == "SCALAR") {


                            for ( int i = 0; i < channelData.outputCount * 4; i++) {
                                float val;
                                std::memcpy(&val, outputDataPtr + (i * 4), 4);
                                auto kfc = animation->keyFrameChannels[i];
                                kfc->value_f = val;;


                            }
                        }
                    }
                }

            auto type_to_string_func = getStringForChannelType;

            std::cout << "animations log:" << std::endl;
            for (auto a : animations) {
                std::cout << "animation name: " << a->name << std::endl;
                for (auto kfc : a->keyFrameChannels) {
                    std::cout << "time: " << kfc->time << std::endl;
                    std::cout << "joint: " << kfc->joint_name << std::endl;
                    std::cout << "type: " <<  type_to_string_func(kfc->type) << std::endl;
                    std::cout << "value_f: " << std::to_string(kfc->value_f) << std::endl;
                    std::cout << "value_v3: " << std::to_string(kfc->value_v3.x) << "/" << std::to_string(kfc->value_v3.y) << std::to_string(kfc->value_v3.z) << std::endl;
                    std::cout << "value_quat: " << kfc->value_quat.w  << "/" << kfc->value_quat.x << "/" << kfc->value_quat.y << "/" << kfc->value_quat.y << std::endl;
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
        // Currently only one texture set supported.
        auto uvIndex = attrObj["TEXCOORD_0"].get<int>();
        auto normalIndex =attrObj["NORMAL"].get<int>();
        // Currently we only support one skin.
        auto jointIndicesIndex = -1;
        if (attrObj.contains("JOINTS_0")) {
            jointIndicesIndex = attrObj["JOINTS_0"].get<int>();
        }

        auto jointWeightsIndex = -1;
        if (attrObj.contains("WEIGHTS_0")) {
            jointWeightsIndex = attrObj["WEIGHTS_0"].get<int>();
        }

        auto indicesIndex = primitivesNode["indices"].get<int>();



        auto posAccessor = gltf[json::json_pointer("/accessors/" + std::to_string(posIndex))];
        auto uvAccessor =gltf[json::json_pointer("/accessors/" + std::to_string(uvIndex))];
        auto normalAccessor = gltf[json::json_pointer("/accessors/" + std::to_string(normalIndex))];
        auto indicesAccessor = gltf[json::json_pointer("/accessors/" + std::to_string(indicesIndex))];
        std::optional<json> jointIndicesAccessorOpt = std::nullopt;
        if (jointIndicesIndex > -1) {
            jointIndicesAccessorOpt = gltf[json::json_pointer("/accessors/" + std::to_string(jointIndicesIndex))];
        }
        std::optional<json> jointWeightsAccessorOpt = std::nullopt;
        if (jointWeightsIndex > -1) {
            jointWeightsAccessorOpt = gltf[json::json_pointer("/accessors/" + std::to_string(jointWeightsIndex))];
        }


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

        auto jointIndicesByteOffset = -1;
        if (jointIndicesAccessorOpt.has_value()) {
            auto jointIndicesViewIndex = jointIndicesAccessorOpt->get<json>()["bufferView"].get<int>();
            auto jointIndicesView = gltf[json::json_pointer("/bufferViews/" + std::to_string(jointIndicesViewIndex))];
            jointIndicesByteOffset = jointIndicesView["byteOffset"].get<int>();
        }


        auto jointWeightsByteOffset = -1;
        if (jointWeightsAccessorOpt.has_value()) {
            auto jointWeightsViewIndex = jointWeightsAccessorOpt->get<json>()["bufferView"].get<int>();
            auto jointWeightsView = gltf[json::json_pointer("/bufferViews/" + std::to_string(jointWeightsViewIndex))];
            jointWeightsByteOffset = jointWeightsView["byteOffset"].get<int>();
        }



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
        auto posData = reinterpret_cast<const float*>(posBase);
        size_t totalFloats = posAccessor["count"].get<int>() * 3;

        std::vector<glm::vec3> positions;
        for (int i = 0; i< totalFloats; i +=3) {
            positions.push_back({posData[i], posData[i+1], posData[i+2]});
        }

        const uint8_t* uvBase = dataBinary.data() + uvBufferByteOffset;
        auto uvData = reinterpret_cast<const float*>(uvBase);
        size_t totalUvs = uvAccessor["count"].get<int>() * 2;
        std::vector<glm::vec2> uvs;
        for (int i = 0; i< totalUvs; i += 2) {
            uvs.push_back({uvData[i], uvData[i+1]});
        }

        const uint8_t* normalBase = dataBinary.data() + normalBufferByteOffset;
        auto normalData = reinterpret_cast<const float*>(normalBase);
        size_t totalNormals = normalAccessor["count"].get<int>() * 3;
        std::vector<glm::vec3> normals;
        for (int i = 0; i < totalNormals; i+= 3) {
            normals.push_back({normalData[i], normalData[i+1], normalData[i+2]});
        }

        std::vector<glm::vec4> jointIndices;
        if (jointIndicesByteOffset > -1) {
            const uint8_t* jointIndexBase = dataBinary.data() + jointIndicesByteOffset;
            const uint8_t* jointIndexData = jointIndexBase;
            size_t totalJointIndicesValues = normalAccessor["count"].get<int>() * 4;

            for (int i = 0; i < totalJointIndicesValues; i+=4) {
                jointIndices.push_back({jointIndexData[i], jointIndexData[i+1], jointIndexData[i+2], jointIndexData[i+3]});
            }
        }

        std::vector<glm::vec4> jointWeights;
        if (jointWeightsByteOffset > -1) {
            const uint8_t* jointWeightsBase = dataBinary.data() + jointWeightsByteOffset;
            const float* jointWeightData = reinterpret_cast<const float*>(jointWeightsBase);
            size_t totalJointWeightValues = normalAccessor["count"].get<int>() * 4;

            for (int i = 0; i < totalJointWeightValues; i+=4) {
                jointWeights.push_back({jointWeightData[i], jointWeightData[i+1], jointWeightData[i+2], jointWeightData[i+3]});

                // Do the weights sum up to 1?
                auto sum = jointWeightData[i] + jointWeightData[i+1] + jointWeightData[i+2] + jointWeightData[i+3];
                // This assertions is a bit strict, GLTF only guarantees to be very close to 1, but we leave it for now.
                assert( sum >= 0.99f && sum <= 1.02f);
            }
        }

        // Now push all the data into one single vector:
        // Positions | UVs
        // Later potentially adding normals etc.
        for (int i = 0; i < positions.size(); i++) {
            ci.data.push_back(positions[i].x);
            ci.data.push_back(positions[i].y * 1 );     // TODO: We should only do this *-1 in vulkan, which we are not aware of here
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
        mesh.joint_weights = jointWeights;
        mesh.joint_indices = jointIndices;
        mesh.positions = positions;
        mesh.uvs = uvs;
        mesh.normals = normals;
        mesh.vertex_data = ci.data;
        mesh.animations = animations;
        mesh.skeleton = skeleton;
        return mesh;

    }
}

std::vector<KeyFrameChannel*> getKeyFramesForJoint(SkeletalAnimation* animation, const std::string& jointName, ChannelType type) {
    std::vector<KeyFrameChannel*> filteredKeyFrames;
    if (type == ChannelType::rotation) {

        for (auto kfc : animation->keyFrameChannels_rotation ) {
            if (kfc->joint_name == jointName) {
                filteredKeyFrames.push_back(kfc);
            }
        }
    }

    return filteredKeyFrames;
}

std::string getStringForChannelType(ChannelType type) {
    switch (type) {
        case ChannelType::translation: return "translation";
        case ChannelType::scale: return "scale";
        case ChannelType::rotation: return "rotation";
    }
    return "";
}
