#pragma once

#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include "graphics/shader.h"
#include <glad/glad.h>
#include <cuda_gl_interop.h>

template<typename DataType>
class BaseRenderer {
protected:
    Shader shader;
    std::vector<DataType> data;
    unsigned int vao{};
    unsigned int vbo_data{};
    unsigned int vbo_base_mesh{};  // optional, some renderers don't use this
    unsigned int buffer_size{};
    cudaGraphicsResource_t cuda_resource{nullptr};
    
    // Virtual methods for customization by derived classes
    virtual void setup_vertex_attributes() = 0;
    virtual void setup_base_mesh() {} // optional override for renderers that use base mesh
    virtual void render_impl(size_t count) = 0;
    virtual size_t get_element_count() const = 0;
    virtual size_t get_bytes_per_element() const = 0;
    virtual bool uses_cuda() const { return true; }
    virtual bool uses_base_mesh() const { return true; }
    virtual const char* get_renderer_name() const = 0;
    
    // Must be called by derived class constructor after virtual table is set up
    void initialize();
    
public:
    BaseRenderer(const char* vertex_path, const char* fragment_path);
    virtual ~BaseRenderer();
    
    void begin();
    void end();
    void render();
    void render(size_t count);
    void set_transform(glm::mat4 transform);
    
    // CUDA interop functions
    void cuda_register_buffer();
    void cuda_unregister_buffer();  
    void* cuda_map_buffer();
    void cuda_unmap_buffer();
    void ensure_vbo_capacity(size_t elements);

private:
    void check_cuda_error(const std::string& operation);
};

// Template implementation (must be in header)
template<typename DataType>
BaseRenderer<DataType>::BaseRenderer(const char* vertex_path, const char* fragment_path) 
    : shader(vertex_path, fragment_path) {
    
    // Create VAO and VBOs
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo_data);
    
    if (uses_base_mesh()) {
        glGenBuffers(1, &vbo_base_mesh);
    }
    
    // Setup data buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo_data);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

template<typename DataType>
void BaseRenderer<DataType>::initialize() {
    if (uses_base_mesh()) {
        setup_base_mesh();
    }
    
    // Setup vertex attributes
    glBindVertexArray(vao);
    setup_vertex_attributes();
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Don't register CUDA buffer yet - wait until it has actual size
}

template<typename DataType>
BaseRenderer<DataType>::~BaseRenderer() {
    if (uses_cuda() && cuda_resource != nullptr) {
        cudaGraphicsUnregisterResource(cuda_resource);
    }
    
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo_data);
    
    if (uses_base_mesh()) {
        glDeleteBuffers(1, &vbo_base_mesh);
    }
}

template<typename DataType>
void BaseRenderer<DataType>::begin() {
    data.clear();
}

template<typename DataType>
void BaseRenderer<DataType>::end() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo_data);
    unsigned int data_bytes = data.size() * sizeof(DataType);
    
    if (data_bytes > buffer_size) {
        // Resize buffer - double it
        buffer_size = data_bytes * 2;
        glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
        
        if (uses_cuda()) {
            cuda_register_buffer();
        }
        
        std::cout << "Doubled " << get_renderer_name() << " buffer size to " << buffer_size << std::endl;
    } else if (data_bytes > 0 && uses_cuda() && cuda_resource == nullptr) {
        // Need to allocate buffer first if buffer_size > 0 but no actual GL buffer allocated
        if (buffer_size > 0) {
            glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
        }
        
        // Register CUDA buffer for the first time when we have data
        cuda_register_buffer();
    }
    
    if (data_bytes > 0) {
        glBufferSubData(GL_ARRAY_BUFFER, 0, data_bytes, data.data());
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template<typename DataType>
void BaseRenderer<DataType>::render() {
    size_t count = data.size() / get_element_count();
    
    if (count == 0) {
        return;
    }
    
    shader.use();
    glBindVertexArray(vao);
    render_impl(count);
    glBindVertexArray(0);
}

template<typename DataType>
void BaseRenderer<DataType>::render(size_t count) {
    shader.use();
    glBindVertexArray(vao);
    render_impl(count);
    glBindVertexArray(0);
}

template<typename DataType>
void BaseRenderer<DataType>::set_transform(glm::mat4 transform) {
    shader.use();
    shader.setMatrix4("transform", transform);
}

template<typename DataType>
void BaseRenderer<DataType>::cuda_register_buffer() {
    if (!uses_cuda()) return;
    
    // Check actual OpenGL buffer size
    glBindBuffer(GL_ARRAY_BUFFER, vbo_data);
    GLint actual_size = 0;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &actual_size);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    if (actual_size == 0) {
        return;
    }
    
    // Unregister if already registered
    if (cuda_resource != nullptr) {
        cudaGraphicsUnregisterResource(cuda_resource);
        cuda_resource = nullptr;
    }
    
    auto error = cudaGraphicsGLRegisterBuffer(&cuda_resource, vbo_data, cudaGraphicsMapFlagsWriteDiscard);
    if (error != cudaSuccess) {
        std::cerr << get_renderer_name() << ": cudaGraphicsGLRegisterBuffer failed: " 
                  << cudaGetErrorString(error) << std::endl;
    }
    check_cuda_error("cuda_register_buffer");
}

template<typename DataType>
void BaseRenderer<DataType>::cuda_unregister_buffer() {
    if (!uses_cuda() || cuda_resource == nullptr) return;
    
    cudaGraphicsUnregisterResource(cuda_resource);
    cuda_resource = nullptr;
    check_cuda_error("cuda_unregister_buffer");
}

template<typename DataType>
void* BaseRenderer<DataType>::cuda_map_buffer() {
    if (!uses_cuda()) return nullptr;
    
    void* device_ptr;
    size_t size;
    cudaGraphicsMapResources(1, &cuda_resource, 0);
    cudaGraphicsResourceGetMappedPointer(&device_ptr, &size, cuda_resource);
    check_cuda_error("cuda_map_buffer");
    return device_ptr;
}

template<typename DataType>
void BaseRenderer<DataType>::cuda_unmap_buffer() {
    if (!uses_cuda()) return;
    
    cudaGraphicsUnmapResources(1, &cuda_resource, 0);
    check_cuda_error("cuda_unmap_buffer");
}

template<typename DataType>
void BaseRenderer<DataType>::ensure_vbo_capacity(size_t elements) {
    const auto size_bytes = elements * get_bytes_per_element();
    
    if (buffer_size < size_bytes) {
        if (buffer_size == 0) {
            buffer_size = size_bytes;
        } else {
            while (buffer_size < size_bytes) {
                buffer_size *= 2;
            }
        }
        
        std::cout << get_renderer_name() << " buffer size changed to " << buffer_size << std::endl;
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo_data);
        glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        if (uses_cuda() && size_bytes > 0) {
            cuda_register_buffer();
        }
    } else if (buffer_size > size_bytes * 4 && size_bytes > 0) {
        buffer_size = size_bytes;
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo_data);
        glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        if (uses_cuda()) {
            cuda_register_buffer();
        }
    }
    
    // Register CUDA if needed (buffer exists but CUDA resource not registered)
    if (uses_cuda() && cuda_resource == nullptr && buffer_size > 0) {
        cuda_register_buffer();
    }
}

template<typename DataType>
void BaseRenderer<DataType>::check_cuda_error(const std::string& operation) {
    if (!uses_cuda()) return;
    
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        std::cerr << get_renderer_name() << ": " << operation << ": " << cudaGetErrorString(err) << std::endl;
    }
}