#ifndef MAT4_H
#define MAT4_H

#include <array>
#include <iostream>
#include <cmath>

// Matrice 4x4 pour transformations 3D (stockage column-major)
class Mat4 {
private:
    std::array<std::array<float, 4>, 4> m;
public:
    Mat4();
    explicit Mat4(float diagonal);
    explicit Mat4(const std::array<float, 16>& values);
    Mat4(const Mat4& other) = default;
    Mat4& operator=(const Mat4& other) = default;
    float& operator()(int col, int row);
    const float& operator()(int col, int row) const;
    std::array<float, 4>& operator[](int col);
    const std::array<float, 4>& operator[](int col) const;
    bool operator==(const Mat4& other) const;
    bool operator!=(const Mat4& other) const;
    Mat4 operator*(const Mat4& other) const;
    Mat4& operator*=(const Mat4& other);
    static Mat4 Identity();
    static Mat4 Translation(float x, float y, float z);
    static Mat4 Scale(float x, float y, float z);
    static Mat4 RotationX(float angle);
    static Mat4 RotationY(float angle);
    static Mat4 RotationZ(float angle);
    static Mat4 Perspective(float fov, float aspect, float near, float far);
    static Mat4 Orthographic(float left, float right, float bottom, float top, float near, float far);
    void Print() const;
};

#endif
