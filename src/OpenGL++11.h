/*
 *
 * OpenGL++11.h = STL + Polymorphism + RAII + thin OpenGL wrapper for C++11
 *
 */
#ifndef OPENGL11_H
#define OPENGL11_H
#include <array>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <GLXW/glxw.h>
#include <GL/gl.h>
#include <lodepng.h>
// vector & matrix containers
#ifdef USE_GLM
#include <glm.h>
#endif
#ifdef USE_ARMADILLO
#include <armadillo>
#endif
#ifdef USE_BOOST
#include <boost/multi_array.h>
#endif

#define GL_CHECK_ERROR() OpenGL11::throwOnGLError(__FILE__, __LINE__)

namespace OpenGL11 {

#ifdef USE_ARMADILLO
typedef arma::fmat::fixed<2, 2> fmat2;
typedef arma::fmat::fixed<3, 3> fmat3;
typedef arma::fmat::fixed<4, 4> fmat4;
typedef arma::fvec::fixed<1> fvec1;
typedef arma::fvec::fixed<2> fvec2;
typedef arma::fvec::fixed<3> fvec3;
typedef arma::fvec::fixed<4> fvec4;
typedef arma::ivec::fixed<1> ivec1;
typedef arma::ivec::fixed<2> ivec2;
typedef arma::ivec::fixed<3> ivec3;
typedef arma::ivec::fixed<4> ivec4;
#endif

inline void throwGLError(const std::string filename, const int line, GLenum err) {
    std::stringstream s_error;
    s_error << filename << ":" << line << ": OpenGL Error (";
    switch(err) {
     case GL_INVALID_OPERATION:
      s_error << "INVALID_OPERATION";
      break;
     case GL_INVALID_ENUM:
      s_error << "INVALID_ENUM";
      break;
     case GL_INVALID_VALUE:
      s_error << "INVALID_VALUE";
      break;
     case GL_OUT_OF_MEMORY:
      s_error << "OUT_OF_MEMORY";
      break;
     case GL_INVALID_FRAMEBUFFER_OPERATION:
      s_error << "INVALID_FRAMEBUFFER_OPERATION: status ";
      GLint fb, status;
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fb);
      switch (status = glCheckFramebufferStatus(fb)) {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
         s_error << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
         break;
         /*
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
         s_error << "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
         break;
         */
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
         s_error << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
         break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
         s_error << "GL_FRAMEBUFFER_UNSUPPORTED";
         break;
        case GL_FRAMEBUFFER_COMPLETE:
         s_error << "GL_FRAMEBUFFER_COMPLETE";
         break;
        default:
         s_error << status;
      }
      break;
     default:
      s_error << "error code " << err ; 
    }
    s_error << ")";
    throw std::logic_error(s_error.str());
}

inline void throwOnGLError(const std::string filename, const int line) {
  GLenum err(glGetError());
  while (err!=GL_NO_ERROR) {
    throwGLError(filename, line, err);
  }
}

class Renderbuffer {
  private:
    GLuint _id;
    GLenum _internalFormat;
  public:
    Renderbuffer(GLenum internalFormat) : _id(0), _internalFormat(internalFormat) { };
    Renderbuffer(GLenum internalFormat, int w, int h) : _id(0), _internalFormat(internalFormat) {
      allocate(w, h);
    };
    ~Renderbuffer() {
      release();
    };
    GLuint id() const {
      return _id;
    };
    bool isCreated() const {
      return (_id != 0);
    };
    void create() {
      glGenRenderbuffers(1, &_id);
      GL_CHECK_ERROR();
    };
    void release() {
      glDeleteRenderbuffers(1, &_id);
      GL_CHECK_ERROR();
    };
    void bind() {
      glBindRenderbuffer(GL_RENDERBUFFER, _id);
      GL_CHECK_ERROR();
    };
    void unbind() const {
      glBindRenderbuffer(GL_RENDERBUFFER, 0);
      GL_CHECK_ERROR();
    };
    void allocate(int w, int h) {
      if(!isCreated()) {
        create();
      }
      bind();
      glRenderbufferStorage(GL_RENDERBUFFER, _internalFormat, w, h);
      GL_CHECK_ERROR();
      unbind();
    };
};

class Texture2D {
  private:
    GLuint _id;
    GLenum _internalFormat;
    unsigned int _w, _h;
  public:
    Texture2D(GLenum pixelFormat) : _id(0), _internalFormat(pixelFormat), _w(0), _h(0) { };
    Texture2D(GLenum pixelFormat, int w, int h, void *data, GLenum dataType = GL_UNSIGNED_BYTE) : _id(0), _internalFormat(pixelFormat), _w(w), _h(h) {
      allocate(w, h, data, dataType);
    };
    Texture2D(GLenum pixelFormat, const std::string filename) : _id(0), _internalFormat(pixelFormat), _w(0), _h(0) {
      loadImage(filename);
    };
    ~Texture2D() {
      release();
    };
    GLuint id() {
      return _id;
    };
    int isCreated() {
      return (_id != 0);
    };
    void create() {
      glGenTextures(1, &_id);
      GL_CHECK_ERROR();
    };
    void release() {
      glDeleteTextures(1, &_id);
      GL_CHECK_ERROR();
    };
    void bind() {
      glBindTexture(GL_TEXTURE_2D, _id);
      GL_CHECK_ERROR();
    };
    void unbind() {
      glBindTexture(GL_TEXTURE_2D, 0);
      GL_CHECK_ERROR();
    };
    void getImage(GLvoid *img, GLenum dataType = GL_UNSIGNED_BYTE, int level = 0) {
      bind();
      glGetTexImage(GL_TEXTURE_2D, level, getFormat(), dataType, img); 
      GL_CHECK_ERROR();
      unbind();
    };
    void saveImage(const std::string filename, int level = 0) {
      std::vector<uint8_t> img(_w * _h * 4);
      getImage(img.data(), GL_UNSIGNED_BYTE, level);
      unsigned int error = lodepng::encode(filename, img, _w, _h);
      if (error) {
        std::cout << "png write error: " << error << std::endl;
      }
    }
    void loadImage(const std::string filename, int level = 0) {
        unsigned int w, h;
        std::vector<uint8_t> img;
        unsigned int error = lodepng::decode(img, w, h, filename);
        if (error) {
          std::cout << "png read error: " << error << std::endl;
        } else {
          allocate(w, h, img.data(), GL_UNSIGNED_BYTE, level);
        }
    }
    void setParameter(GLenum pname, GLint param) {
      bind();
      glTexParameteri(GL_TEXTURE_2D, pname, param);
      GL_CHECK_ERROR();
      unbind();
    };
    void setParameter(GLenum pname, GLfloat param) {
      bind();
      glTexParameterf(GL_TEXTURE_2D, pname, param);
      GL_CHECK_ERROR();
      unbind();
    };
    void allocate(int w, int h, void *data = NULL, GLenum dataType = GL_UNSIGNED_BYTE, int level = 0) {
      if(!isCreated()) {
        create();
      }
      bind();
      _w = w;
      _h = h;
      glTexImage2D(GL_TEXTURE_2D, level, _internalFormat, w, h, /* border - "must be 0." */ 0, getFormat(), dataType, data);
      GL_CHECK_ERROR();
      unbind();
    };
    /* get "base internal format" from "sized internal format" */
    GLenum getFormat() {
      switch(_internalFormat) {
        /* breakthrough */
        case GL_R8_SNORM:
        case GL_R8:
        case GL_R16:
        case GL_R16F:
        case GL_R32F:
        case GL_R8I:
        case GL_R16I:
        case GL_R32I:
        case GL_R8UI:
        case GL_R16UI:
        case GL_R32UI:
          return GL_RED;
        case GL_RG8:
        case GL_RG16:
        case GL_RG16F:
        case GL_RG32F:
        case GL_RG8I:
        case GL_RG16I:
        case GL_RG32I:
        case GL_RG8UI:
        case GL_RG16UI:
        case GL_RG32UI:
        case GL_RG8_SNORM:
          return GL_RG;
        case GL_RGB8:
        case GL_RGB9_E5:
        case GL_RGB16F:
        case GL_RGB32F:
        case GL_RGB32I:
        case GL_RGB32UI:
          return GL_RGB;
        case GL_RGBA8:
        case GL_RGBA16:
        case GL_RGBA16F:
        case GL_RGBA32F:
        case GL_RGBA8I:
        case GL_RGBA16I:
        case GL_RGBA32I:
        case GL_RGBA8UI:
        case GL_RGBA16UI:
        case GL_RGBA32UI:
        case GL_SRGB8_ALPHA8:
          return GL_RGBA;
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32F:
          return GL_DEPTH_COMPONENT;
        case GL_DEPTH24_STENCIL8:
          return GL_DEPTH_STENCIL;
      }
      return 0;
    };

    size_t pixel_depth () {
      int count;

      switch(getFormat()) {
        case GL_R:
        case GL_DEPTH_COMPONENT:
          count = 1;
          break;
        case GL_RG:
          count = 2;
          break;
        case GL_RGB:
          count = 3;
          break;
        case GL_RGBA:
          count = 4;
          break;
        default:
          count = 0;
      }

      switch(_internalFormat) {
        /* breakthrough */
        case GL_RGB9_E5:
        case GL_DEPTH24_STENCIL8:
          return 4;

        case GL_R8:
        case GL_R8I:
        case GL_R8UI:
        case GL_R8_SNORM:
        case GL_RG8:
        case GL_RG8I:
        case GL_RG8UI:
        case GL_RG8_SNORM:
        case GL_RGB8:
        case GL_RGBA8:
        case GL_RGBA8I:
        case GL_RGBA8UI:
        case GL_SRGB8_ALPHA8:
          return count;

        case GL_R16:
        case GL_R16F:
        case GL_R16I:
        case GL_R16UI:
        case GL_RG16:
        case GL_RG16F:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RGB16F:
        case GL_RGBA16:
        case GL_RGBA16F:
        case GL_RGBA16I:
        case GL_RGBA16UI:
        case GL_DEPTH_COMPONENT16:
          return 2 * count;

        case GL_DEPTH_COMPONENT24:
          return 3 * count;

        case GL_R32F:
        case GL_R32I:
        case GL_R32UI:
        case GL_RG32F:
        case GL_RG32I:
        case GL_RG32UI:
        case GL_RGB32F:
        case GL_RGB32I:
        case GL_RGB32UI:
        case GL_RGBA32F:
        case GL_RGBA32I:
        case GL_RGBA32UI:
        case GL_DEPTH_COMPONENT32F:
          return 4 * count;
        default:
          std::stringstream s_error;
          s_error << "unknown internal format: " << (int) _internalFormat;
          throw std::logic_error(s_error.str());
      }
    }
};

class Framebuffer {
  private:
    GLuint _id;
  public:
    Framebuffer() : _id(0) { };
    ~Framebuffer() { release(); };
    GLuint id() {
      return _id;
    };
    int isCreated() {
      return (_id != 0);
    };
    void create() {
      glGenFramebuffers(1, &_id);
      GL_CHECK_ERROR();
    };
    void release() {
      glDeleteFramebuffers(1, &_id);
      GL_CHECK_ERROR();
    };
    void bind() {
      if (!isCreated()) {
        create();
      }
      glBindFramebuffer(GL_FRAMEBUFFER, _id);
      GL_CHECK_ERROR();
    };
    void unbind() {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      GL_CHECK_ERROR();
    };
    void attach() {};
    template <typename... Args>
      void attach(GLenum attachmentType, Texture2D &texture, Args&&... args) {
        bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               attachmentType,
                               GL_TEXTURE_2D,
                               texture.id(), 0);
        GL_CHECK_ERROR();
        attach(args...);
        unbind();
      };
    template <typename... Args>
      void attach(GLenum attachmentType, Renderbuffer &renderBuffer, Args... args) {
        bind();
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, renderBuffer.id());
        GL_CHECK_ERROR();
        attach(args...);
        unbind();
      };
    void detach(GLenum attachmentType) {
        bind();
        /* attach texture id 0 */
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               attachmentType,
                               GL_TEXTURE_2D,
                               0, 0);
        GL_CHECK_ERROR();
        unbind();
    };
};

class VertexArray {
  private:
    GLuint _id;
  public:
    VertexArray() : _id(0) {};
    ~VertexArray() { release(); };
    GLuint id() {
      return _id;
    };
    bool isCreated() {
      return (_id != 0);
    };
    void create() {
      glGenVertexArrays(1, &_id);
      GL_CHECK_ERROR();
    };
    void release() {
      glDeleteVertexArrays(1, &_id);
      GL_CHECK_ERROR();
    };
    void bind() {
      if (!isCreated()) {
        create();
      }
      glBindVertexArray(_id);
      GL_CHECK_ERROR();
    };
    void unbind() {
      glBindVertexArray(0);
      GL_CHECK_ERROR();
    };

};

template <typename T> class Buffer {
  private:
    GLuint _id;
    GLsizei _tupleSize;
    GLenum _dataType, _bufferType, _usage;
    bool _isMutable;
  public:
    Buffer(GLenum a_bufferType, GLenum a_usage = GL_STATIC_DRAW, bool a_mutable = true, GLenum a_dataType = 0)
      : _id(0), _bufferType(a_bufferType), _usage(a_usage), _isMutable(a_mutable) {
        T tmp_t(0);
        if (a_dataType) {
          _dataType = a_dataType;
        } else {
          setDataType(tmp_t);
        }
      };
    ~Buffer() {
      release();
    };
    GLuint id() {
      return _id;
    };
    bool isCreated() {
      return (_id != 0);
    };
    GLenum bufferType () {
      return _bufferType;
    };
    GLenum usage () {
      return _usage;
    };
    GLenum dataType () {
      return _dataType;
    };
    GLsizei tupleSize() {
      return _tupleSize;
    };
    void create() {
      glGenBuffers(1, &_id);
      GL_CHECK_ERROR();
    };
    void release() {
      glDeleteBuffers(1, &_id);
      GL_CHECK_ERROR();
    };
    void bind() {
      if (!isCreated()) {
        create();
      }
      glBindBuffer(_bufferType, _id);
      GL_CHECK_ERROR();
    };
    void unbind() {
      glBindBuffer(_bufferType, 0);
      GL_CHECK_ERROR();
    };
    void setDataType(GLfloat) {
      _dataType = GL_FLOAT;
    };
    void setDataType(GLdouble) {
      _dataType = GL_DOUBLE;
    };
    void setDataType(GLubyte) {
      _dataType = GL_UNSIGNED_BYTE;
    };
    void setDataType(GLbyte) {
      _dataType = GL_BYTE;
    };
    void setDataType(GLshort) {
      _dataType = GL_SHORT;
    };
    void setDataType(GLushort) {
      _dataType = GL_UNSIGNED_SHORT;
    };
    void setDataType(GLint) {
      _dataType = GL_INT;
    };
    void setDataType(GLuint) {
      _dataType = GL_UNSIGNED_INT;
    };
    void allocate(T *data, int a_tupleSize, int size) {
      if (!isCreated()) {
        create();
      }
      _tupleSize = a_tupleSize;
      bind();
      if (_isMutable) {
        glBufferData(_bufferType, size, data, _usage);
        GL_CHECK_ERROR();
      } else {
        glBufferStorage(_bufferType, size, data, _usage);
        GL_CHECK_ERROR();
      }
      unbind();
    };
    template <int i>
      void allocate(std::array<T, i> array, int a_tupleSize) {
        allocate(array.data(), a_tupleSize, sizeof(T) * i);
      };
    void allocate(std::vector<T> vec, int a_tupleSize) {
      allocate(vec.data(), a_tupleSize, sizeof(T) * vec.size());
    };
#ifdef USE_BOOST
    template <int i, int j>
    void allocate(boost::multi_array<T, 2> array) {
      allocate(array.data(), array.shape()[1], sizeof(T) * array.num_elements());
    }
#endif
  };


class Shader {
  private:
    GLuint _id;
    GLenum _type;
  public:
    Shader(GLuint type) : _id(0), _type(type) {};
    ~Shader() { release(); };
    GLuint id() {
      return _id;
    };
    bool isCreated() {
      return (_id != 0);
    }
    void create() {
      _id = glCreateShader(_type);
      GL_CHECK_ERROR();
    };
    void release() {
      glDeleteShader(_id);
      GL_CHECK_ERROR();
    };
    void setSourceString(std::string source) {
      const GLchar *gl_source;
      GLsizei length = (GLsizei) source.length();
      gl_source = (GLchar *) source.c_str();
      if (!isCreated()) {
        create();
      }
      glShaderSource(_id, 1, &gl_source, &length);
      GL_CHECK_ERROR();
    };
    void compile() {
      if (!isCreated()) {
        create();
      }
      glCompileShader(_id);
      GL_CHECK_ERROR();
      GLint success = 0;
      glGetShaderiv(_id, GL_COMPILE_STATUS, &success);
      if (success == GL_FALSE) {
        std::cout << infolog() << std::endl;
        throw std::runtime_error("shader compile failed");
      }
    };
    void compileFromSource(std::string source) {
      setSourceString(source);
      compile();
    };
    void compileFromSourceFile(std::string filename) {
      std::ifstream file;
      std::stringstream sourceStream;
      file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
      file.open(filename);
      sourceStream << file.rdbuf();
      compileFromSource(sourceStream.str());
    };
    std::string infolog() {
      GLint len;
      GLsizei written;
      GLchar *log;
      if (!isCreated()) {
        create();
      }
      glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &len);
      GL_CHECK_ERROR();
      if (len > 0) {
        log = (GLchar *) malloc(len);
        glGetShaderInfoLog(_id, (GLsizei) len, &written, log);
        GL_CHECK_ERROR();
        return (char *) log;
      }
      return "";
    };
};

class ShaderProgram {
  private:
    GLuint _id;
    int texture_unit_number;
  public:
    ShaderProgram() : _id(0), texture_unit_number(0) {};
    ~ShaderProgram() { release(); };
    GLuint id() {
      return _id;
    };
    bool isCreated() {
      return (_id != 0);
    }
    void create() {
      _id = glCreateProgram();
      GL_CHECK_ERROR();
    };
    void release() {
      glDeleteProgram(_id);
      GL_CHECK_ERROR();
    };
    void bind() {
      texture_unit_number = 0;
      if (!isCreated()) {
        create();
      }
      glUseProgram(_id);
      GL_CHECK_ERROR();
    };
    void unbind() {
      glUseProgram(0);
      GL_CHECK_ERROR();
    };
    void setAttributeBuffer(const char * name, GLenum type, const intptr_t offset, GLsizei tuple) {
      if (!isCreated()) {
        create();
      }
      glVertexAttribPointer(glGetAttribLocation(_id, name), tuple, type, GL_TRUE, 0, (GLvoid *) offset);
      GL_CHECK_ERROR();
    };
    void setUniformValue (const char * name, Texture2D &texture) {
        int max_texture_units;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);
        GL_CHECK_ERROR();
        if (texture_unit_number >= max_texture_units) {
          std::stringstream error;
          error << "Cannot bind more than " << max_texture_units << " textures at one time";
          throw std::runtime_error(error.str());
        }
        glActiveTexture(GL_TEXTURE0 + texture_unit_number);
        GL_CHECK_ERROR();
        texture.bind();
        setUniformValue(name, texture_unit_number);
        texture_unit_number++;
    };
    template <typename... Args>
      void bind(VertexArray &vao, Args&&... args) {
        vao.bind();
        bind(args ...);
      };
    template <typename... Args, typename T>
      void bind(const char * name, Buffer<T>& buf, const intptr_t offset, Args&&... args) {
        bind(args ...);
        buf.bind();
        enableAttributeArray(name);
        setAttributeBuffer(name, buf.dataType(), offset, buf.tupleSize());
      }
    template <typename... Args, typename T>
      void bind(const char * name, Buffer<T>& buf, Args&&... args) {
        bind(args ...);
        buf.bind();
        enableAttributeArray(name);
        setAttributeBuffer(name, buf.dataType(), 0, buf.tupleSize());
      }
    template <typename... Args>
      void bind(const char * name, const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w, Args&&... args) {
        bind(args ...);
        setUniformValue(name, x, y, z, w);
      }
    template <typename... Args>
      void bind(const char * name, const GLfloat x, const GLfloat y, const GLfloat z, Args&&... args) {
        bind(args ...);
        setUniformValue(name, x, y, z);
      }
    template <typename... Args>
      void bind(const char * name, const GLfloat x, const GLfloat y, Args&&... args) {
        bind(args ...);
        setUniformValue(name, x, y);
      }
    template <typename T, typename... Args>
      void bind(const char * name, const T *value, int count, int tuple, Args&&... args) {
        bind(args ...);
        setUniformValueArray(name, value, count, tuple);
      }
    template <typename T, typename... Args>
      void bind(const char * name, const T *value, int count, Args&&... args) {
        bind(args ...);
        setUniformValue(name, value, count);
      }
    template <typename T, typename... Args>
      void bind(const char * name, T& value, Args&&... args) {
        bind(args ...);
        setUniformValue(name, value);
      };

    void attach(Shader& s) {
      if (!isCreated()) {
        create();
      }
      glAttachShader(_id, s.id());
      GL_CHECK_ERROR();
    };

    void detach(Shader& s) {
      glDetachShader(_id, s.id());
      GL_CHECK_ERROR();
    };

    void link() {
      if (!isCreated()) {
        create();
      }
      glLinkProgram(_id);
      GL_CHECK_ERROR();
    };

    template <typename... Args>
      void link(GLenum type, const char* filename, Args&&... args) {
        Shader s(type);
        s.compileFromSourceFile(filename);
        link(s, args...);
      };
    template <typename... Args>
      void link(const char* filename, Args&&... args) {
        std::cout << "compiling: " << filename << std::endl;
        GLenum type;
        std::string s_filename = filename, extension = "";
        if (s_filename.find_last_of(".") != std::string::npos) {
          extension = s_filename.substr(s_filename.find_last_of("."));
        }
        if (extension == ".vert" || extension == ".glslv") {
          type = GL_VERTEX_SHADER;
        } else if (extension == ".tesc" || extension == ".tsc") {
          type = GL_TESS_CONTROL_SHADER;
        } else if (extension == ".tese" || extension == ".tse") {
          type = GL_TESS_EVALUATION_SHADER;
        } else if (extension == ".geom" || extension == ".glslg") {
          type = GL_GEOMETRY_SHADER;
        } else if (extension == ".frag" || extension == ".glslf") {
          type = GL_FRAGMENT_SHADER;
        } else if (extension == ".comp"|| extension == ".glslc") {
          type = GL_COMPUTE_SHADER;
        } else {
          throw std::runtime_error("cannot detect the shader type of: " + s_filename);
        }
        Shader s(type);
        s.compileFromSourceFile(filename);
        link(s, args...);
      };
    template <typename... Args>
      void link(Shader& s, Args&&... args) {
        attach(s);
        link(args ...);
        detach(s);
      };

    void enableAttributeArray(GLuint loc) {
      glEnableVertexAttribArray(loc);
      GL_CHECK_ERROR();
    };
    void enableAttributeArray(const char *locName) {
      if (!isCreated()) {
        create();
      }
      glEnableVertexAttribArray(glGetAttribLocation(_id, locName));
      GL_CHECK_ERROR();
    };
    template <typename... Args>
      void setUniformValue (const char *locName, Args&&... args) {
       if (!isCreated()) {
         create();
       }
       setUniformValue((GLuint) glGetUniformLocation(_id, locName), args...);
      };
#ifdef USE_ARMADILLO
    inline void setUniformValue (GLuint loc, const fvec1& s) { glUniform1fv(loc, 1, (GLfloat *) s.memptr()); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, const fvec2& s) { glUniform2fv(loc, 1, (GLfloat *) s.memptr()); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, const fvec3& s) { glUniform3fv(loc, 1, (GLfloat *) s.memptr()); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, const fvec4& s) { glUniform4fv(loc, 1, (GLfloat *) s.memptr()); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, const ivec1& s) { glUniform1iv(loc, 1, (GLint *) s.memptr()); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, const ivec2& s) { glUniform2iv(loc, 1, (GLint *) s.memptr()); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, const ivec3& s) { glUniform3iv(loc, 1, (GLint *) s.memptr()); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, const ivec4& s) { glUniform4iv(loc, 1, (GLint *) s.memptr()); GL_CHECK_ERROR(); };
    // armadillo stores data in a column by column order
    inline void setUniformValue (GLuint loc, const fmat2& s) { glUniformMatrix2fv(loc, 1, GL_FALSE, (GLfloat *) s.memptr()); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, const fmat3& s) { glUniformMatrix3fv(loc, 1, GL_FALSE, (GLfloat *) s.memptr()); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, const fmat4& s) { glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat *) s.memptr()); GL_CHECK_ERROR(); };
#endif
#ifdef USE_GLM
    inline void setUniformValue (GLuint loc, const glm::vec1& s) { glUniform1fv(loc, 1, (GLfloat *) glm::value_ptr(s); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, const glm::vec2& s) { glUniform2fv(loc, 1, (GLfloat *) glm::value_ptr(s); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, const glm::vec3& s) { glUniform3fv(loc, 1, (GLfloat *) glm::value_ptr(s); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, const glm::vec4& s) { glUniform4fv(loc, 1, (GLfloat *) glm::value_ptr(s); GL_CHECK_ERROR(); };
    // GLM stores data in a column by column order
    inline void setUniformValue (GLuint loc, const glm::mat2& s) { glUniformMatrix2fv(loc, 1, GL_FALSE, (GLfloat *) glm::value_ptr(s); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, const glm::mat3& s) { glUniformMatrix3fv(loc, 1, GL_FALSE, (GLfloat *) glm::value_ptr(s); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, const glm::mat4& s) { glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat *) glm::value_ptr(s); GL_CHECK_ERROR(); };
#endif
    inline void setUniformValue (GLuint loc, GLfloat s) { glUniform1f(loc, s); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, GLfloat s, GLfloat t) { glUniform2f(loc, s, t); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, GLfloat s, GLfloat t, GLfloat u) { glUniform3f(loc, s, t, u); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, GLfloat s, GLfloat t, GLfloat u, GLfloat v) { glUniform4f(loc, s, t, u, v); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, GLint s)   { glUniform1i(loc, s); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, GLint s, GLint t) { glUniform2i(loc, s, t); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, GLint s, GLint t, GLint u) { glUniform3i(loc, s, t, u); GL_CHECK_ERROR(); };
    inline void setUniformValue (GLuint loc, GLint s, GLint t, GLint u, GLint v) { glUniform4i(loc, s, t, u, v); GL_CHECK_ERROR(); };
};
};
#endif
