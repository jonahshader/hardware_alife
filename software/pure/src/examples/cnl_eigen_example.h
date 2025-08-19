#pragma once

#include <cnl/scaled_integer.h>
#include <cnl/fixed_point.h>
#include <Eigen/Dense>
#include <iostream>

namespace cnl_eigen_examples {

// Define a scaled integer type with 16 bits and 8 fractional bits
using scaled_int16_8 = cnl::scaled_integer<std::int16_t, cnl::power<-8>>;

// Define a fixed-point type with 32 bits and 16 fractional bits  
using fixed_point32_16 = cnl::fixed_point<std::int32_t, -16>;

// 2D vector using CNL scaled integers
using Vector2s = Eigen::Vector2<scaled_int16_8>;

// 2D vector using CNL fixed-point
using Vector2f = Eigen::Vector2<fixed_point32_16>;

// 3D vector using CNL scaled integers
using Vector3s = Eigen::Vector3<scaled_int16_8>;

// Matrix using CNL types
using Matrix2s = Eigen::Matrix2<scaled_int16_8>;

inline void demonstrate_cnl_eigen_usage() {
    std::cout << "=== CNL + Eigen Examples ===\n";
    
    // Create 2D vectors with scaled integers
    Vector2s v1(scaled_int16_8{1.5}, scaled_int16_8{2.25});
    Vector2s v2(scaled_int16_8{0.5}, scaled_int16_8{1.75});
    
    std::cout << "Vector v1: (" << static_cast<float>(v1.x()) << ", " << static_cast<float>(v1.y()) << ")\n";
    std::cout << "Vector v2: (" << static_cast<float>(v2.x()) << ", " << static_cast<float>(v2.y()) << ")\n";
    
    // Vector operations
    Vector2s v3 = v1 + v2;
    std::cout << "v1 + v2 = (" << static_cast<float>(v3.x()) << ", " << static_cast<float>(v3.y()) << ")\n";
    
    // Dot product
    auto dot_product = v1.dot(v2);
    std::cout << "v1 · v2 = " << static_cast<float>(dot_product) << "\n";
    
    // Norm (magnitude)
    auto norm = v1.norm();
    std::cout << "||v1|| = " << static_cast<float>(norm) << "\n";
    
    // Matrix operations
    Matrix2s m1;
    m1 << scaled_int16_8{1.0}, scaled_int16_8{0.5},
          scaled_int16_8{0.5}, scaled_int16_8{1.0};
    
    Vector2s result = m1 * v1;
    std::cout << "Matrix * v1 = (" << static_cast<float>(result.x()) << ", " << static_cast<float>(result.y()) << ")\n";
    
    // Using fixed-point for higher precision
    Vector2f vf1(fixed_point32_16{1.5}, fixed_point32_16{2.25});
    Vector2f vf2(fixed_point32_16{0.5}, fixed_point32_16{1.75});
    
    Vector2f vf3 = vf1 + vf2;
    std::cout << "\nFixed-point vectors:\n";
    std::cout << "vf1 + vf2 = (" << static_cast<double>(vf3.x()) << ", " << static_cast<double>(vf3.y()) << ")\n";
    
    auto fp_dot = vf1.dot(vf2);
    std::cout << "vf1 · vf2 = " << static_cast<double>(fp_dot) << "\n";
}

// Example of a simple physics simulation using CNL + Eigen
struct Particle {
    Vector2s position;
    Vector2s velocity;
    scaled_int16_8 mass;
    
    Particle(scaled_int16_8 x, scaled_int16_8 y, scaled_int16_8 vx, scaled_int16_8 vy, scaled_int16_8 m)
        : position(x, y), velocity(vx, vy), mass(m) {}
    
    void update(scaled_int16_8 dt) {
        position += velocity * dt;
    }
    
    void apply_force(const Vector2s& force, scaled_int16_8 dt) {
        Vector2s acceleration = force / mass;
        velocity += acceleration * dt;
    }
};

inline void demonstrate_physics_simulation() {
    std::cout << "\n=== Physics Simulation Example ===\n";
    
    Particle p(scaled_int16_8{0.0}, scaled_int16_8{0.0},    // position
               scaled_int16_8{1.0}, scaled_int16_8{0.5},    // velocity  
               scaled_int16_8{2.0});                        // mass
    
    Vector2s gravity(scaled_int16_8{0.0}, scaled_int16_8{-9.8});
    scaled_int16_8 dt{0.016}; // ~60 FPS
    
    std::cout << "Initial: pos=(" << static_cast<float>(p.position.x()) << ", " << static_cast<float>(p.position.y()) << ")\n";
    
    // Simulate a few time steps
    for (int i = 0; i < 5; ++i) {
        p.apply_force(gravity * p.mass, dt);
        p.update(dt);
        
        std::cout << "Step " << i+1 << ": pos=(" << static_cast<float>(p.position.x()) << ", " 
                  << static_cast<float>(p.position.y()) << "), vel=(" << static_cast<float>(p.velocity.x()) 
                  << ", " << static_cast<float>(p.velocity.y()) << ")\n";
    }
}

} // namespace cnl_eigen_examples