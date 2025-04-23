//
// Created by mgrus on 23.04.2025.
//

#include <Windows.h>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <cstdio>
#include <stdexcept>
#include <symbol_exports.h>
#include "../include/opengl46.h"

#include <iostream>
#include <unordered_map>

#include "../renderer/include/renderer.h"

// Handle system
uint32_t nextHandleId = 1; // start at 1 to reserve 0 as "invalid"
static std::unordered_map<uint32_t, GLTexture> textureMap;
static std::unordered_map<uint32_t, GLShader> shaderMap;
static std::unordered_map<uint32_t, GLProgram> programMap;
static std::unordered_map<uint32_t, GLVao> vaoMap;
static std::unordered_map<uint32_t, GLVbo> vbufferMap;

renderer::FragmentShaderBuilder & GL46FramgentShaderBuilder::color() {
    useColor = true;
    return *this;
}

std::string GL46FramgentShaderBuilder::build() const {
    std::string src = "#version 460 core\n";

    if (useColor)
        src += "uniform vec4 color = vec4(1, 1, 1, 1);\n";

    src += "out vec4 final_color;\n";

    src += "void main() {\n";

    if (useColor)
        src += "    final_color = color;\n";
    else
        src += "    final_color = vec4(1, 0,1, 1);\n";

    src += "}\n";

    return src;

}




template<typename T>
renderer::VertexBufferBuilder & GL46VertexBufferBuilder::attributeT(renderer::VertexAttributeSemantic semantic,
    const std::vector<T> &data) {
    size_t offset = current_stride_;
    current_stride_ += sizeof(glm::vec3);

    auto attr = renderer::VertexAttribute();
    attr.semantic = semantic;
    attr.offset = static_cast<uint32_t>(offset);
    attr.format = renderer::VertexFormatFor<T>::value;
    attr.stride = sizeof(T);
    attr.location = attributes_.size();
    attr.components = renderer::ComponentsFor<T>::value;

    attributes_.push_back({attr});
    raw_data_.insert(raw_data_.end(), reinterpret_cast<const float*>(data.data()),
        reinterpret_cast<const float*>(data.data()) + data.size()*3);
    element_size_ = sizeof(glm::vec3); // maybe verify consistency?
    element_count_ = data.size();
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

renderer::VertexBufferHandle GL46VertexBufferBuilder::build() const {
    //std::vector<float> interleavedData(element_count_ * current_stride_);

    // for (size_t i = 0; i < element_count_; ++i) {
    //     for (size_t j = 0; j < raw_data_.size(); ++j) {
    //         const float* src = &raw_data_[j] + i *  element_size_;
    //         void* dst = interleavedData.data() + i * current_stride_ + attributes_[j].offset;
    //         std::memcpy(dst, src, element_size_);
    //     }
    // }



    renderer::VertexBufferCreateInfo info = {
        .data = raw_data_,
        .stride = current_stride_
    };

    return createVertexBuffer(info);
}

renderer::IndexBufferHandle renderer::createIndexBuffer(std::vector<uint32_t> data) {
    GLuint ibo;
    glCreateBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(uint32_t), data.data(), GL_STATIC_DRAW);
    auto handleId = nextHandleId++;
    vbufferMap[handleId] = GLVbo{ibo};
    return renderer::IndexBufferHandle{handleId};
}



renderer::VertexBufferHandle renderer::createVertexBuffer(renderer::VertexBufferCreateInfo create_info) {
    GLuint vbo;
    glCreateBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, create_info.data.size() * sizeof(float), create_info.data.data(), GL_STATIC_DRAW);
    auto handleId = nextHandleId++;
    vbufferMap[handleId] = GLVbo{vbo};
    return renderer::VertexBufferHandle{handleId};

}

void initOpenGL46(HWND hwnd) {

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
        WGL_SAMPLE_BUFFERS_ARB, 1,      // <-- Enable multi-sample buffer
        WGL_SAMPLES_ARB,        4,      // <-- # of samples
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
    glEnable(GL_MULTISAMPLE);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST );
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_FRAMEBUFFER_SRGB);
    //glEnable(GL_POLYGON_SMOOTH);
    //glViewport(0, 0, w, h);

    auto enableVsync = [](bool enable) {
        ((BOOL(WINAPI*)(int))wglGetProcAddress("wglSwapIntervalEXT"))(enable);
    };
    enableVsync(false);

    GLint samples = 0, sampleBuffers = 0;
    glGetIntegerv(GL_SAMPLES, &samples);
    glGetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);
    printf("MSAA: %d sample buffers, %d samples\n", sampleBuffers, samples);

}

namespace renderer {


    void ENGINE_API present(HDC hdc) {
        SwapBuffers(hdc);
    }

    void ENGINE_API clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    std::unique_ptr<VertexBufferBuilder> vertexBufferBuilder() {
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

    void drawMesh(Mesh m) {
        glBindVertexArray(m.id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbufferMap[m.index_buffer.id].id);
        glDrawElements(GL_TRIANGLES, m.index_count, GL_UNSIGNED_INT, 0);
    }

    void bindProgram(ProgramHandle myprog) {
        glUseProgram(programMap[myprog.id].id);
    }


    Mesh renderer::createMesh(VertexBufferHandle vbo, IndexBufferHandle ibo, const std::vector<VertexAttribute> &attributes, size_t index_count) {
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        for (auto attribute : attributes) {
            glEnableVertexAttribArray(attribute.location);
            glVertexAttribPointer(
                attribute.location,               // attribute location
                attribute.components,               // components (vec3)
                GL_FLOAT,        // type
                GL_FALSE,        // normalize?
                attribute.stride,
                (void*)attribute.offset                // relative offset in vertex
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

     std::unique_ptr<VertexShaderBuilder>  vertexShaderBuilder() {
        return std::make_unique<GL46VertexShaderBuilder>();
    }

    std::unique_ptr<FragmentShaderBuilder> fragmentShaderBuilder() {
        return std::make_unique<GL46FramgentShaderBuilder>();
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
        ShaderHandle handle = ShaderHandle {nextHandleId};
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


}