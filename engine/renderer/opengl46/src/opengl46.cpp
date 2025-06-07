//
// Created by mgrus on 23.04.2025.
//

#include <Windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cstdio>
#include <stdexcept>
#include "../include/opengl46.h"

#include <complex>
#include <fstream>
#include <iostream>
#include <Joint.h>
#include <ranges>
#include <unordered_map>

#include <nljson.h>
#include <skeleton.h>

#include <renderer.h>
#include "base64.h"


// Handle system
uint32_t nextHandleId = 1; // start at 1 to reserve 0 as "invalid"
static std::unordered_map<uint32_t, GLTexture> textureMap;
static std::unordered_map<uint32_t, GLShader> shaderMap;

static std::unordered_map<uint32_t, GLProgram> programMap;
static std::unordered_map<uint32_t, GLVao> vaoMap;
static std::unordered_map<uint32_t, GLVbo> vbufferMap;

namespace renderer {
    static TextureHandle createTextureGL46(const Image& image, TextureFormat format);
    static std::unique_ptr<VertexBufferBuilder> vertexBufferBuilderGL46();
    static IndexBufferHandle createIndexBufferGL46(const IndexBufferDesc&);
    static void updateIndexBufferGL46(IndexBufferHandle iboHandle, std::vector<uint32_t> data);
    static Mesh createMeshGL46(VertexBufferHandle vbo, IndexBufferHandle ibo, const std::vector<VertexAttribute> & attributes, size_t index_count);
    static Mesh importMeshGL46(const std::string& filename);
}


renderer::RenderTargetBuilder & GL46RenderTargetBuilder::size(int w, int h) {
    width = w;
    height = h;
    return *this;
}

renderer::RenderTargetBuilder & GL46RenderTargetBuilder::color() {
    usesColor = true;
    return *this;
}

renderer::RenderTargetBuilder & GL46RenderTargetBuilder::depth() {
    useDepth = true;
    return *this;
}

renderer::RenderTargetBuilder & GL46RenderTargetBuilder::stencil() {
    useStencil = true;
    return *this;
}

renderer::RenderTarget GL46RenderTargetBuilder::build() {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    renderer::RenderTarget renderTarget;
    renderTarget.id = fbo;
    renderTarget.width = width;
    renderTarget.height = height;

    if (usesColor) {
        GLuint colorTexture;
        glGenTextures(1, &colorTexture);
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
        GLTexture glTex;
        glTex.id = colorTexture;
        glTex.width = width;
        glTex.height = height;
        auto textureHandle = renderer::TextureHandle {nextHandleId};
        textureMap[textureHandle.id] = glTex;
        nextHandleId++;
        renderTarget.colorTex = textureHandle;
    }

    if (useDepth) {
        GLuint depthRbo;
        glGenRenderbuffers(1, &depthRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRbo);
        renderTarget.depthRbo = depthRbo;
    }

    if (useStencil) {
        GLuint stencilRbo;
        glGenRenderbuffers(1, &stencilRbo);
        glBindRenderbuffer(GL_RENDERBUFFER, stencilRbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
        glFramebufferRenderbuffer(GL_RENDERBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRbo);
        renderTarget.stencilRbo = stencilRbo;
    }

    // Check completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        // log error!
    }

    // Unbind:
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    return renderTarget;

}



template<typename T>
renderer::VertexBufferBuilder & GL46VertexBufferBuilder::attributeT(renderer::VertexAttributeSemantic semantic,
    const std::vector<T> &data) {
    size_t offset = current_stride_;
    current_stride_ += sizeof(T);

    auto attr = renderer::VertexAttribute();
    attr.semantic = semantic;
    attr.offset = static_cast<uint32_t>(offset);
    attr.format = renderer::VertexFormatFor<T>::value;
    attr.stride = current_stride_;
    attr.location = attributes_.size();
    attr.components = renderer::ComponentsFor<T>::value;

    attributes_.push_back({attr});
    const float* float_data = reinterpret_cast<const float*>(data.data());
    raw_data_.insert(raw_data_.end(), float_data, float_data + data.size() * T::length());
    element_count_ = data.size(); // how many elements (e.g. positions) do we have here?
    return *this;
}


renderer::VertexBufferBuilder & GL46VertexBufferBuilder::attributeVec3(renderer::VertexAttributeSemantic semantic,
    const std::vector<glm::vec3> &data) {
    return attributeT(semantic, data);
}

renderer::VertexBufferBuilder & GL46VertexBufferBuilder::attributeVec2(renderer::VertexAttributeSemantic semantic,
    const std::vector<glm::vec2> &data) {
    return attributeT(semantic, data);
}

template<>
renderer::VertexBufferCreateInfo<float> GL46VertexBufferBuilder::commonUpdateBuild() const {
    std::vector<float> interleavedData(element_count_ * (current_stride_ / sizeof(float)));

    for (size_t i = 0; i < element_count_; ++i) {
        size_t attrOffsetFloats = 0;
        size_t attrDataOffset = 0;

        for (size_t j = 0; j < attributes_.size(); ++j) {
            const auto& attr = attributes_[j];

            size_t attrSizeFloats = attr.components;
            size_t dstOffset = i * (current_stride_ / sizeof(float)) + (attr.offset / sizeof(float));
            size_t srcOffset = i * attrSizeFloats + attrDataOffset;

            std::memcpy(&interleavedData[dstOffset], &raw_data_[srcOffset], attrSizeFloats * sizeof(float));
            attrDataOffset += element_count_ * attrSizeFloats; // shift to next attribute's chunk
        }
    }


    renderer::VertexBufferCreateInfo info = {
        .data = interleavedData,
        .stride = current_stride_
    };

    return info;
}

renderer::VertexBufferHandle GL46VertexBufferBuilder::build() const {
    auto info = commonUpdateBuild<float>();
    return createVertexBuffer(info);
}

void GL46VertexBufferBuilder::update(renderer::VertexBufferHandle oldVbo) const {
    auto createInfo  = commonUpdateBuild<float>();
    renderer::VertexBufferUpdateInfo updateInfo = {
        .data = createInfo.data,
        .stride = createInfo.stride,
        .oldVBO = oldVbo
    };
    updateVertexBuffer(updateInfo);
}

void renderer::updateVertexBuffer(renderer::VertexBufferUpdateInfo updateInfo) {
    GLuint vbo = vbufferMap[updateInfo.oldVBO.id].id;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, updateInfo.data.size() * sizeof(float), updateInfo.data.data(), GL_DYNAMIC_DRAW);
}



template<typename T>
renderer::VertexBufferHandle renderer::createVertexBuffer(renderer::VertexBufferCreateInfo<T> create_info) {
    GLuint vbo;
    glCreateBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    auto size =create_info.data.size() * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, size , create_info.data.data(), GL_DYNAMIC_DRAW);
    auto handleId = nextHandleId++;
    vbufferMap[handleId] = GLVbo{vbo};
    return renderer::VertexBufferHandle{handleId};

}

void initOpenGL46(HWND hwnd, bool useSRGB, int msaaSampleCount) {

    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    // 1. Register a dummy window class
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style         = CS_OWNDC;                  // get our own DC for the window
    wc.lpfnWndProc   = DefWindowProcA;            // a trivial wndproc
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = "Dummy_WGL";

    if (!RegisterClassA(&wc)) {
        // handle error
    }

    // 2. Create the hidden (dummy) window
    HWND dummyWnd = CreateWindowA(
        "Dummy_WGL",            // class name
        "Dummy OpenGL Window",  // window name
        WS_OVERLAPPEDWINDOW,    // style
        CW_USEDEFAULT, CW_USEDEFAULT, // position
        CW_USEDEFAULT, CW_USEDEFAULT, // size
        NULL,                   // parent
        NULL,                   // menu
        wc.hInstance,
        NULL
    );
    if (!dummyWnd) {
        // TODO handle error
        exit(999);
    }

    // 3. Get the DC for the dummy window
    HDC dummyDC = GetDC(dummyWnd);

    int windowsChosenFormat = ChoosePixelFormat(dummyDC, &pfd);
    SetPixelFormat(dummyDC, windowsChosenFormat, &pfd);

    HGLRC baseContext = wglCreateContext(dummyDC);
    BOOL ok = wglMakeCurrent (dummyDC, baseContext);
    if (!ok) {
        printf("error");
        exit(1);
    }

    int gl46_attribs[] =
            {
                    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                    WGL_CONTEXT_MINOR_VERSION_ARB, 6,
                    WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                    0,
            };


    //HGLRC wglCreateContextAttribsARB(HDC hDC, HGLRC hShareContext, const int *attribList)
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    auto createContextAttribsProc = (PFNWGLCREATECONTEXTATTRIBSARBPROC ) wglGetProcAddress("wglCreateContextAttribsARB");

    int pixelAttribs[] =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,     32,
        WGL_DEPTH_BITS_ARB,     24,
        WGL_STENCIL_BITS_ARB,   8,
        WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE, // <--- Enable sRGB framebuffer
        WGL_SAMPLE_BUFFERS_ARB, msaaSampleCount > 0 ? 1 : 0,      // <-- Enable multi-sample buffer
        WGL_SAMPLES_ARB,        msaaSampleCount,      // <-- # of samples
        0                       // End
    };

    int pixelFormat;
    UINT numFormats; // will receive how many formats match

    // Find a pixel format that matches our criteria
    HDC hdc_ = GetDC(hwnd);
    bool success = wglChoosePixelFormatARB(hdc_, pixelAttribs, nullptr, 1,
                                           &pixelFormat, &numFormats);

    if ( ! success || numFormats ==0 ) {
        // TODO handle error, no throw in dll exports.
        //throw std::runtime_error("Failed to create OpenGL context with MSAA 4x");
    }

    HDC ourWindowHandleToDeviceContext = GetDC(hwnd);
    SetPixelFormat(ourWindowHandleToDeviceContext, pixelFormat, &pfd);

    auto coreRenderContext = createContextAttribsProc(hdc_, nullptr, gl46_attribs);
    wglDeleteContext(baseContext);
    ok = wglMakeCurrent(ourWindowHandleToDeviceContext, coreRenderContext);
    if (!ok) {
        printf("error");
        exit(1);
    }

    // Initialize GLEW
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        // GLEW initialization failed
        fprintf(stderr, "Error initializing GLEW: %s\n", glewGetErrorString(err));
        exit(1);
    }


    const GLubyte *GLVersionString = glGetString(GL_VERSION);
    char buf[200];
    sprintf_s(buf, "gl version: %s\n", GLVersionString);
    OutputDebugStringA(buf);
    printf("%s", buf);

#ifdef RENDER2D
    // Setup the fullscreen texture
    BITMAPINFO bbInfo;
    bbInfo.bmiHeader.biBitCount = 32;
    bbInfo.bmiHeader.biCompression = BI_RGB;
    bbInfo.bmiHeader.biPlanes = 1;
    bbInfo.bmiHeader.biHeight = window_height;
    bbInfo.bmiHeader.biWidth = window_width;
    bbInfo.bmiHeader.biSize = sizeof(bbInfo.bmiHeader);
    bbInfo.bmiHeader.biSizeImage = 0;
    bbInfo.bmiHeader.biXPelsPerMeter = 0;
    bbInfo.bmiHeader.biYPelsPerMeter = 0;
    bbInfo.bmiHeader.biClrUsed = 0;
    bbInfo.bmiHeader.biClrImportant = 0;

    int bytesPerPixel = 4;
    int bitmapMemorySize = bbInfo.bmiHeader.biWidth * bbInfo.bmiHeader.biHeight * bytesPerPixel;
    backbufferBytes = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, window_width, window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, backbufferBytes);

    // Onetime ortho projection setup
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float ar = (float) window_height / (float) window_width;
    glOrtho(0, window_width, 0, window_height, 0, 1);
    glViewport(0, 0, window_width, window_height);
    glEnable(GL_TEXTURE_2D);

    auto err = glGetError();
    if (err != 0) {
        printf("gl error: %d\n", err);
        exit(err);
    }

#endif

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);


    if (msaaSampleCount > 0) {
        glEnable(GL_MULTISAMPLE);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST );
        glEnable(GL_LINE_SMOOTH);
    }



    if (useSRGB) {
        glEnable(GL_FRAMEBUFFER_SRGB);
    }

    //glEnable(GL_POLYGON_SMOOTH);
    RECT r;
    GetClientRect(hwnd, &r);
    glViewport(0, 0, r.right-r.left, r.bottom-r.top);

    auto enableVsync = [](bool enable) {
        ((BOOL(WINAPI*)(int))wglGetProcAddress("wglSwapIntervalEXT"))(enable);
    };
    enableVsync(false);

    GLint samples = 0, sampleBuffers = 0;
    glGetIntegerv(GL_SAMPLES, &samples);
    glGetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);
    printf("MSAA: %d sample buffers, %d samples\n", sampleBuffers, samples);

    registerCreateTexture(&renderer::createTextureGL46);
    registerVertexBufferBuilder(&renderer::vertexBufferBuilderGL46);
    registerCreateIndexBuffer(&renderer::createIndexBufferGL46);
    registerCreateMesh(&renderer::createMeshGL46);
    registerImportMesh(&renderer::importMeshGL46);
    renderer::registerUpdateIndexBuffer(&renderer::updateIndexBufferGL46);


}

namespace renderer {


    void present(HDC hdc) {
        SwapBuffers(hdc);
    }

    void clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    std::unique_ptr<VertexBufferBuilder> vertexBufferBuilderGL46() {
        return std::make_unique<GL46VertexBufferBuilder>();
    }

    ProgramHandle createShaderProgram(const char *vertexShader, const char *fragmentShader) {
        auto vs = glCreateShader(GL_VERTEX_SHADER);

        auto fs = glCreateShader(GL_FRAGMENT_SHADER);
        auto prog = glCreateProgram();


        GLProgram glProg;
        glProg.id = prog;
        auto id = nextHandleId;
        programMap[id] = glProg;
        nextHandleId++;
        return ProgramHandle{id};

    }



    static GLint toInternalGLFormat(TextureFormat format) {
        switch (format) {
            case TextureFormat::SRGBA8: return GL_SRGB8_ALPHA8;
            case TextureFormat::RGBA8: return GL_RGBA8;
            case TextureFormat::R8 : return GL_R8;
            default: return GL_RGBA8;
        }
    }

    static GLenum toGLFormat(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGBA8: return GL_RGBA;
            case TextureFormat::R8: return GL_RED;
            case TextureFormat::SRGBA8: return GL_RGBA;
            default: return GL_RGBA;
        }
    }

    TextureHandle createTextureGL46(const Image& image, TextureFormat format) {

        GLuint handle;
        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_2D, handle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     toInternalGLFormat(format),
                     image.width,
                     image.height,
                     0,
                     toGLFormat(format),
                     GL_UNSIGNED_BYTE,
                     image.pixels);

        glGenerateMipmap(GL_TEXTURE_2D);

        GLTexture glTex;
        glTex.id = handle;
        glTex.width = image.width;
        glTex.height = image.height;
        //glTex.format = textureFormat;
        auto textureHandle = TextureHandle {nextHandleId};
        textureMap[textureHandle.id] = glTex;
        nextHandleId++;
        return textureHandle;

        // // Set aniso filtering
        // {
        //     if (!glewIsSupported("GL_EXT_texture_filter_anisotropic")) {
        //         throw std::runtime_error("Anisotropic filtering not supported!");
        //     }
        //
        //     GLfloat maxAniso = 0.0f;
        //     glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        //     printf("maxAniso: %f\n", maxAniso);
        //     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
        //
        // }

    }

    void bindTexture(TextureHandle texture) {
        // TODO logic for the correct texture unit
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureMap[texture.id].id);
    }

    void drawMesh(Mesh m, const std::string& debugInfo) {
        if (!debugInfo.empty()) {
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, debugInfo.c_str());
        }
        glBindVertexArray(m.id);
        auto index_buffer = vbufferMap[m.index_buffer.id];
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.id);
        glDrawElements(GL_TRIANGLES, m.index_count, index_buffer.format, 0);
        if (!debugInfo.empty()) {
            glPopDebugGroup();
        }
    }



    void bindProgram(ProgramHandle myprog) {
        glUseProgram(programMap[myprog.id].id);
    }






    std::unique_ptr<RenderTargetBuilder> renderTargetBuilder() {
        return std::make_unique<GL46RenderTargetBuilder>();
    }

    void bindRenderTarget(const RenderTarget &renderTarget) {
        glBindFramebuffer(GL_FRAMEBUFFER, renderTarget.id);
    }

    void bindDefaultRenderTarget() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }



    ShaderHandle compileFragmentShader(const std::string &source) {
        GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
        const GLchar* fssource_char = source.c_str();
        glShaderSource(fshader, 1, &fssource_char, NULL);
        glCompileShader(fshader);

        GLint compileStatus;
        glGetShaderiv(fshader, GL_COMPILE_STATUS, &compileStatus);
        if (GL_FALSE == compileStatus) {
            GLint logSize = 0;
            glGetShaderiv(fshader, GL_INFO_LOG_LENGTH, &logSize);
            std::vector<GLchar> errorLog(logSize);
            glGetShaderInfoLog(fshader, logSize, &logSize, &errorLog[0]);
            //   result.errorMessage = errorLog.data();
            printf("fragment shader error: %s", errorLog.data());
            glDeleteShader(fshader);

        }

        auto glShader = GLShader{fshader};
        auto handle = ShaderHandle {nextHandleId};
        shaderMap[nextHandleId] = glShader ;
        nextHandleId++;
        return handle;
    }

    ProgramHandle linkShaderProgram(ShaderHandle vshader, ShaderHandle fshader) {
        GLuint p = glCreateProgram();
        glAttachShader(p, shaderMap[vshader.id].id);
        glAttachShader(p, shaderMap[fshader.id].id);
        glLinkProgram(p);

        GLint linkStatus;
        glGetProgramiv(p, GL_LINK_STATUS, &linkStatus);

        if (GL_FALSE == linkStatus) {
            std::cerr << "Error during shader linking" << std::endl;
            GLint maxLength = 0;
            glGetProgramiv(p, GL_INFO_LOG_LENGTH, &maxLength);
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(p, maxLength, &maxLength, &infoLog[0]);
            std::cerr << infoLog.data() << std::endl;
            printf("shader linking error: %s", infoLog.data());
            glDeleteProgram(p);
        }

        auto glProg = GLProgram{p};
        ProgramHandle handle = ProgramHandle {nextHandleId};
        programMap[nextHandleId] = glProg ;
        nextHandleId++;
        return handle;

    }

    ShaderHandle compileVertexShader(const std::string &source) {
        auto vshader = glCreateShader(GL_VERTEX_SHADER);
        const GLchar* vssource_char = source.c_str();
        glShaderSource(vshader, 1, &vssource_char, NULL);
        glCompileShader(vshader);
        GLint compileStatus;
        glGetShaderiv(vshader, GL_COMPILE_STATUS, &compileStatus);
        if (GL_FALSE == compileStatus) {
            std::cerr << "Error while compiling the vertex shader" << std::endl;

            GLint logSize = 0;
            glGetShaderiv(vshader, GL_INFO_LOG_LENGTH, &logSize);
            std::vector<GLchar> errorLog(logSize);
            glGetShaderInfoLog(vshader, logSize, &logSize, &errorLog[0]);
            //    result.errorMessage = errorLog.data();
            char buf[512];
            sprintf(buf, "vshader error: %s", errorLog.data());
            printf(buf);
            std::cerr << buf << std::endl;
            glDeleteShader(vshader);
            //  return result;

        }
        auto glShader = GLShader{vshader};
        ShaderHandle handle = ShaderHandle {nextHandleId};
        shaderMap[nextHandleId] = glShader ;
        nextHandleId++;
        return handle;
    }

    void setClearColor(float r, float g, float b, float a) {
        glClearColor(r, g, b, a);
    }

    template<>
    bool setShaderValue<glm::vec2>(ProgramHandle program, const std::string& name, const glm::vec2& value) {
        auto prog = programMap[program.id].id;
        glUseProgram(prog);
        auto loc = glGetUniformLocation(prog, name.c_str());
        glUniform2fv(loc, 1, glm::value_ptr(value));
        return (glGetError() == GL_NO_ERROR);
    }



    template<>
    bool setShaderValue<float>(ProgramHandle program, const std::string& name, const float& value) {
        auto prog = programMap[program.id].id;
        glUseProgram(prog);
        auto loc = glGetUniformLocation(prog, name.c_str());
        glUniform1f(loc, value);
        return (glGetError() == GL_NO_ERROR);
    }

    template<>
    bool setShaderValue<glm::mat4>(ProgramHandle program, const std::string& name, const glm::mat4& value) {
        auto prog = programMap[program.id].id;
        glUseProgram(prog);
        auto loc = glGetUniformLocation(prog, name.c_str());
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
        auto err = glGetError();
        return (err == GL_NO_ERROR) ;
    }

    IndexBufferHandle createIndexBufferGL46(const IndexBufferDesc& desc) {
        GLuint ibo;
        glCreateBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc.size_in_bytes, desc.data, GL_STATIC_DRAW);
        auto handleId = nextHandleId++;
        vbufferMap[handleId] = GLVbo{ibo, desc.format};
        return renderer::IndexBufferHandle{handleId};
    }



    void updateIndexBufferGL46(IndexBufferHandle iboHandle, std::vector<uint32_t> data) {
        GLuint ibo = vbufferMap[iboHandle.id].id;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(uint32_t), data.data(), GL_DYNAMIC_DRAW);
    }

    bool strContains(const std::string& str, char test) {
        for (auto c : str) {
            if (c == test) return true;
        }

        return false;
    }

    bool strContains(const std::string &str, const std::string &test) {
        if (test.length() > str.length()) return false;
        if (test.length() == str.length()) return str == test;

        for (int i = 0; i < str.length(); i++) {
            std::string testSequence = "";
            for (int t = 0; t  < test.length(); t++) {
                testSequence += str[t];
            }
            if (testSequence == test) {
                return true;
            }


        }

        return false;
    }

    struct GlbChunk {
        uint32_t chunkLength;
        uint32_t chunkType;   // 0x4E4F534A = "JSON", 0x004E4942 = "BIN"
        std::vector<uint8_t>  chunkData;
    };



        /**
     * We only accept scenes with 1 mesh for now.
     * @param gltfJson the parsed json of the gltf
     * @return A Pointer to a newly created mesh object.
     */
     static Mesh parseGLTF(const std::string& filename) {

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

        /*
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        GLuint posBuffer;
        glGenBuffers(1, &posBuffer);
        glBindBuffer(posGlTargetType, posBuffer);
        glBufferData(posGlTargetType, posBufferLen, dataBinary.data() + posBufferByteOffset, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        GLuint uvBuffer;
        glGenBuffers(1, &uvBuffer);
        glBindBuffer(uvGlTargetType, uvBuffer);
        glBufferData(uvGlTargetType, uvBufferLen, dataBinary.data() + uvBufferByteOffset , GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        GLuint normalBuffer;
        glGenBuffers(1, &normalBuffer);
        glBindBuffer(normalGlTargetType, normalBuffer);
        glBufferData(normalGlTargetType, normalBufferLen, dataBinary.data() + normalBufferByteOffset, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);

        std::vector<float> instanceOffsets = {0, 0};
        unsigned int instanceVBO;
        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 1, instanceOffsets.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glVertexAttribDivisor(3, 1);

        GLuint indexBuffer;
        glGenBuffers(1, &indexBuffer);
        glBindBuffer(indicesGlTargetType, indexBuffer);
        glBufferData(indicesGlTargetType, indicesBufferLen, dataBinary.data() + indicesBufferByteOffset, GL_STATIC_DRAW);
        */

        VertexBufferCreateInfo<float> ci;
        const uint8_t* base = dataBinary.data() + posBufferByteOffset;
        const float* floatData = reinterpret_cast<const float*>(base);
        size_t totalFloats = posAccessor["count"].get<int>() * 3;
        ci.data =  std::vector(floatData, floatData + totalFloats);
        ci.stride = 0;
        auto vbo = createVertexBuffer(ci);

        const uint8_t* indices = dataBinary.data() + indicesBufferByteOffset;

        IndexBufferDesc ibd;
        ibd.size_in_bytes = indicesBufferLen;
        ibd.format = indexComponentType;
        ibd.data = indices;
        IndexBufferHandle ibo = createIndexBufferGL46(ibd);

        VertexAttribute posAttribute;
        posAttribute.location = 0;
        posAttribute.stride = 0;
        posAttribute.components = 3;
        posAttribute.format = VertexAttributeFormat::Float3;
        posAttribute.offset = 0;
        posAttribute.semantic = VertexAttributeSemantic::Position;
        auto mesh = createMeshGL46(vbo, ibo, {posAttribute}, indexCount);
        return mesh;

    }


    Mesh importMeshGL46(const std::string& filename) {
        return parseGLTF(filename);
    }

    Mesh createMeshGL46(VertexBufferHandle vbo, IndexBufferHandle ibo, const std::vector<VertexAttribute> &attributes,
                       size_t index_count) {
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        for (auto attribute: attributes) {
            glEnableVertexAttribArray(attribute.location);
            glVertexAttribPointer(
                    attribute.location,               // attribute location
                    attribute.components,               // components
                    GL_FLOAT,        // type
                    GL_FALSE,        // normalize?
                    attribute.stride,
                    (void *) attribute.offset                // relative offset in vertex
            );

        }
        glBindVertexArray(0);

        Mesh mesh;
        mesh.id = vao;
        mesh.index_count = index_count;
        mesh.index_buffer = ibo;
        mesh.vertex_buffer = vbo;
        mesh.layout = VertexLayout{attributes};
        mesh.primitive_topology = PrimitiveTopology::Triangle_List;
        return mesh;
    }



    template bool setShaderValue<glm::mat4>(ProgramHandle, const std::string&, const glm::mat4&);
    template bool setShaderValue<float>(ProgramHandle, const std::string&, const float&);
    template bool setShaderValue<glm::vec2>(ProgramHandle, const std::string&, const glm::vec2&);

}