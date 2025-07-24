#pragma once

#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include "graphics/shader.h"
#include <glad/glad.h>

template<typename DataType>
class BaseRenderer {
protected:
    Shader shader;
    std::vector<DataType> data;
    unsigned int vbo_data{};
    unsigned int vbo_base_mesh{};  // optional, some renderers don't use this
    unsigned int buffer_size{};
    
    // Virtual methods for customization by derived classes
    virtual void setup_vertex_attributes() = 0;
    virtual void setup_base_mesh() {} // optional override for renderers that use base mesh
    virtual void render_impl(size_t count) = 0;
    virtual size_t get_element_count() const = 0;
    virtual size_t get_bytes_per_element() const = 0;
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
    void set_screen_scale(float scale = 1.0f);
    
    void ensure_vbo_capacity(size_t elements);
};

// Template implementation (must be in header)
template<typename DataType>
BaseRenderer<DataType>::BaseRenderer(const char* vertex_path, const char* fragment_path) 
    : shader(vertex_path, fragment_path) {
    
    // Create VBOs
    glGenBuffers(1, &vbo_data);
    
    if (uses_base_mesh()) {
        glGenBuffers(1, &vbo_base_mesh);
    }
    
    // Setup data buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo_data);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template<typename DataType>
void BaseRenderer<DataType>::initialize() {
    if (uses_base_mesh()) {
        setup_base_mesh();
    }
    
    // Vertex attributes will be set up during rendering
}

template<typename DataType>
BaseRenderer<DataType>::~BaseRenderer() {
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
        
        std::cout << "Doubled " << get_renderer_name() << " buffer size to " << buffer_size << std::endl;
    } else if (data_bytes > 0 && buffer_size == 0) {
        // Need to allocate buffer first time
        buffer_size = data_bytes;
        glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
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
    setup_vertex_attributes();
    render_impl(count);
    
    // Clean up vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    for (int i = 0; i < 8; i++) {
        glDisableVertexAttribArray(i);
    }
}

template<typename DataType>
void BaseRenderer<DataType>::render(size_t count) {
    shader.use();
    setup_vertex_attributes();
    render_impl(count);
    
    // Clean up vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    for (int i = 0; i < 8; i++) {
        glDisableVertexAttribArray(i);
    }
}

template<typename DataType>
void BaseRenderer<DataType>::set_transform(glm::mat4 transform) {
    shader.use();
    shader.setMatrix4("transform", transform);
}

template<typename DataType>
void BaseRenderer<DataType>::set_screen_scale(float scale) {
    shader.use();
    shader.setFloat("screenScale", scale);
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
    } else if (buffer_size > size_bytes * 4 && size_bytes > 0) {
        buffer_size = size_bytes;
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo_data);
        glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}