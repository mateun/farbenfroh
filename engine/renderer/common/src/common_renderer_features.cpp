//
// Created by mgrus on 25.04.2025.
//
#include "../../include/renderer.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <unordered_map>

struct Font {
    uint32_t id;
    renderer::TextureHandle fontAtlas;
    float maxDescent = std::numeric_limits<float>::max();
    float lineHeight = std::numeric_limits<float>::min();
    std::vector<stbtt_bakedchar> bakedChars;

};

uint32_t nextHandleId = 1; // start at 1 to reserve 0 as "invalid"
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
    static VertexBufferBuilderFn g_vertexBufferBuilder = nullptr;
    static CreateMeshFn g_createMesh = nullptr;

    void registerCreateTexture(CreateTextureFn fn) {
        g_createTexture = fn;
    }

    void registerVertexBufferBuilder(VertexBufferBuilderFn fn) {
        g_vertexBufferBuilder = fn;
    }

    void registerCreateIndexBuffer(CreateIndexBufferFn fn) {
        g_createIndexBuffer = fn;
    }

    void registerCreateMesh(CreateMeshFn fn) {
        g_createMesh = fn;
    }

    TextureHandle createTexture(const Image& image, TextureFormat format) {
        if (!g_createTexture) {
            return {};
        }
        return g_createTexture(image, format);
    }

    IndexBufferHandle createIndexBuffer(std::vector<uint32_t> data) {
        if (!g_createIndexBuffer) {
            return {};
        }
        return g_createIndexBuffer(data);
    }

    std::unique_ptr<VertexBufferBuilder> vertexBufferBuilder() {
        if (!g_vertexBufferBuilder) {
            return {};
        }
        return g_vertexBufferBuilder();
    }

    Mesh createMesh(VertexBufferHandle vbo, IndexBufferHandle ibo, const std::vector<VertexAttribute> & attributes, size_t index_count) {
        if (!g_createMesh) {
            return {};
        }
        return g_createMesh(vbo, ibo, attributes, index_count);
    }

    // TODO: maybe this could go into a "common" renderer implementation.
    // It is not GL46 specific (not GL at all...).
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

    Mesh drawTextIntoQuad(FontHandle fontHandle, const std::string& text) {

        auto font = fontMap[fontHandle.id];


        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<uint32_t> indices;

        float penX = 0, penY = 0;
        float minX =  std::numeric_limits<float>::max();
        float maxX = -std::numeric_limits<float>::max();
        float minY =  std::numeric_limits<float>::max();
        float maxY = -std::numeric_limits<float>::max();
        int charCounter = 0;
        for (auto c : text) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font.bakedChars.data(), 512, 512, c - 32, &penX, &penY, &q, 1);

            float pixel_aligned_x0 = std::floor(q.x0 + 0.5f);
            float pixel_aligned_y0 = std::floor(q.y0 + 0.5f);
            float pixel_aligned_x1 = std::floor(q.x1 + 0.5f);
            float pixel_aligned_y1 = std::floor(q.y1 + 0.5f);

            q.x0 = pixel_aligned_x0;
            q.y0 = -pixel_aligned_y0;
            q.x1 = pixel_aligned_x1;
            q.y1 = -pixel_aligned_y1;

            positions.push_back(glm::vec3(q.x0, q.y0, 0));
            positions.push_back(glm::vec3(q.x1, q.y0, 0));
            positions.push_back(glm::vec3(q.x1, q.y1, 0));
            positions.push_back(glm::vec3(q.x0, q.y1, 0));

            uvs.push_back({q.s0, q.t0});
            uvs.push_back({q.s1, q.t0});
            uvs.push_back({q.s1, q.t1});
            uvs.push_back({q.s0, q.t1});

            int offset = charCounter * 4;
            indices.push_back(2 + offset);indices.push_back(1 + offset);indices.push_back(0 + offset);
            indices.push_back(2 + offset);indices.push_back(0 + offset);indices.push_back(3 + offset);
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

        auto vbo = vertexBufferBuilder()->attributeVec3(VertexAttributeSemantic::Position, positions)
            .attributeVec2(VertexAttributeSemantic::UV0, uvs).build();
        auto ibo = createIndexBuffer(indices);

        std::vector<renderer::VertexAttribute> vertexAttributes = {
            renderer::VertexAttribute{
                .semantic = renderer::VertexAttributeSemantic::Position,
                .format = renderer::VertexAttributeFormat::Float3,
                .location = 0,
                .offset = 0,
                .components = 3,
                .stride = 20
            },
             renderer::VertexAttribute{
                 .semantic = renderer::VertexAttributeSemantic::UV0,
                 .format = renderer::VertexAttributeFormat::Float2,
                 .location = 1,
                 .offset = 12,
                 .components = 2,
                 .stride = 20
             }

        };
        auto quadMesh = createMesh(vbo, ibo, vertexAttributes, indices.size());
        return quadMesh;
    }

}