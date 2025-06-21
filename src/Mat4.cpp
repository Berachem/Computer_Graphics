#include "Mat4.h"
#include <iomanip>
#include <stdexcept>

// === Constructeurs ===

Mat4::Mat4() {
    for (int col = 0; col < 4; ++col)
        for (int row = 0; row < 4; ++row)
            m[col][row] = (col == row) ? 1.0f : 0.0f;
}

Mat4::Mat4(float diagonal) {
    for (int col = 0; col < 4; ++col)
        for (int row = 0; row < 4; ++row)
            m[col][row] = (col == row) ? diagonal : 0.0f;
}

Mat4::Mat4(const std::array<float, 16>& values) {
    int index = 0;
    for (int col = 0; col < 4; ++col)
        for (int row = 0; row < 4; ++row)
            m[col][row] = values[index++];
}

// === Accès aux éléments ===

float& Mat4::operator()(int col, int row) {
    if (col < 0 || col >= 4 || row < 0 || row >= 4)
        throw std::out_of_range("Index de matrice hors limites");
    return m[col][row];
}

const float& Mat4::operator()(int col, int row) const {
    if (col < 0 || col >= 4 || row < 0 || row >= 4)
        throw std::out_of_range("Index de matrice hors limites");
    return m[col][row];
}

std::array<float, 4>& Mat4::operator[](int col) {
    if (col < 0 || col >= 4)
        throw std::out_of_range("Index de colonne hors limites");
    return m[col];
}

const std::array<float, 4>& Mat4::operator[](int col) const {
    if (col < 0 || col >= 4)
        throw std::out_of_range("Index de colonne hors limites");
    return m[col];
}

// === Opérations matricielles ===

Mat4 Mat4::operator*(const Mat4& other) const {
    Mat4 result(0.0f); // Matrice nulle
    
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            for (int k = 0; k < 4; ++k) {
                result.m[col][row] += m[k][row] * other.m[col][k];
            }
        }
    }
    
    return result;
}

Mat4 Mat4::operator*(float scalar) const {
    Mat4 result;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            result.m[col][row] = m[col][row] * scalar;
        }
    }
    return result;
}

Mat4 Mat4::operator+(const Mat4& other) const {
    Mat4 result;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            result.m[col][row] = m[col][row] + other.m[col][row];
        }
    }
    return result;
}

Mat4 Mat4::operator-(const Mat4& other) const {
    Mat4 result;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            result.m[col][row] = m[col][row] - other.m[col][row];
        }
    }
    return result;
}

// === Méthodes utilitaires ===

Mat4 Mat4::transpose() const {
    Mat4 result;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            result.m[col][row] = m[row][col];
        }
    }
    return result;
}

float Mat4::determinant() const {
    float det = 0.0f;
    
    det += m[0][0] * (
        m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
        m[1][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) +
        m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1])
    );
    
    det -= m[1][0] * (
        m[0][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
        m[0][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) +
        m[0][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1])
    );
    
    det += m[2][0] * (
        m[0][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) -
        m[0][2] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) +
        m[0][3] * (m[1][1] * m[3][2] - m[1][2] * m[3][1])
    );
    
    det -= m[3][0] * (
        m[0][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) -
        m[0][2] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) +
        m[0][3] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
    );
    
    return det;
}

const float* Mat4::data() const {
    return &m[0][0];
}

void Mat4::print() const {
    std::cout << std::fixed << std::setprecision(3);
    for (int row = 0; row < 4; ++row) {
        std::cout << "| ";
        for (int col = 0; col < 4; ++col) {
            std::cout << std::setw(8) << m[col][row] << " ";
        }
        std::cout << "|" << std::endl;
    }
    std::cout << std::endl;
}

// === Méthodes statiques ===

Mat4 Mat4::identity() {
    return Mat4();
}

Mat4 Mat4::translate(float x, float y, float z) {
    Mat4 result;
    result.m[3][0] = x;
    result.m[3][1] = y;
    result.m[3][2] = z;
    return result;
}

Mat4 Mat4::rotateX(float angle) {
    Mat4 result;
    float c = std::cos(angle);
    float s = std::sin(angle);
    
    result.m[1][1] = c;
    result.m[1][2] = s;
    result.m[2][1] = -s;
    result.m[2][2] = c;
    
    return result;
}

Mat4 Mat4::rotateY(float angle) {
    Mat4 result;
    float c = std::cos(angle);
    float s = std::sin(angle);
    
    result.m[0][0] = c;
    result.m[0][2] = -s;
    result.m[2][0] = s;
    result.m[2][2] = c;
    
    return result;
}

Mat4 Mat4::rotateZ(float angle) {
    Mat4 result;
    float c = std::cos(angle);
    float s = std::sin(angle);
    
    result.m[0][0] = c;
    result.m[0][1] = s;
    result.m[1][0] = -s;
    result.m[1][1] = c;
    
    return result;
}

Mat4 Mat4::scale(float x, float y, float z) {
    Mat4 result;
    result.m[0][0] = x;
    result.m[1][1] = y;
    result.m[2][2] = z;
    return result;
}

Mat4 Mat4::inverse() const {
    float det = determinant();

    if (std::abs(det) < 1e-6f) {
        std::cout << "Attention: Matrice non inversible, retour de la matrice identité" << std::endl;
        return Mat4::identity();
    }

    Mat4 result;
    float invDet = 1.0f / det;

    result.m[0][0] = invDet * (
        m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
        m[1][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) +
        m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1])
    );

    result.m[0][1] = -invDet * (
        m[0][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
        m[0][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) +
        m[0][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1])
    );

    result.m[0][2] = invDet * (
        m[0][1] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) -
        m[0][2] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) +
        m[0][3] * (m[1][1] * m[3][2] - m[1][2] * m[3][1])
    );

    result.m[0][3] = -invDet * (
        m[0][1] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) -
        m[0][2] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) +
        m[0][3] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
    );

    result.m[1][0] = -invDet * (
        m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
        m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
        m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0])
    );

    result.m[1][1] = invDet * (
        m[0][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
        m[0][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
        m[0][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0])
    );

    result.m[1][2] = -invDet * (
        m[0][0] * (m[1][2] * m[3][3] - m[1][3] * m[3][2]) -
        m[0][2] * (m[1][0] * m[3][3] - m[1][3] * m[3][0]) +
        m[0][3] * (m[1][0] * m[3][2] - m[1][2] * m[3][0])
    );

    result.m[1][3] = invDet * (
        m[0][0] * (m[1][2] * m[2][3] - m[1][3] * m[2][2]) -
        m[0][2] * (m[1][0] * m[2][3] - m[1][3] * m[2][0]) +
        m[0][3] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
    );

    result.m[2][0] = invDet * (
        m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) -
        m[1][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
        m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])
    );

    result.m[2][1] = -invDet * (
        m[0][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) -
        m[0][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
        m[0][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])
    );

    result.m[2][2] = invDet * (
        m[0][0] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) -
        m[0][1] * (m[1][0] * m[3][3] - m[1][3] * m[3][0]) +
        m[0][3] * (m[1][0] * m[3][1] - m[1][1] * m[3][0])
    );

    result.m[2][3] = -invDet * (
        m[0][0] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) -
        m[0][1] * (m[1][0] * m[2][3] - m[1][3] * m[2][0]) +
        m[0][3] * (m[1][0] * m[2][1] - m[1][1] * m[2][0])
    );

    result.m[3][0] = -invDet * (
        m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) -
        m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) +
        m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])
    );

    result.m[3][1] = invDet * (
        m[0][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) -
        m[0][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) +
        m[0][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0])
    );

    result.m[3][2] = -invDet * (
        m[0][0] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]) -
        m[0][1] * (m[1][0] * m[3][2] - m[1][2] * m[3][0]) +
        m[0][2] * (m[1][0] * m[3][1] - m[1][1] * m[3][0])
    );

    result.m[3][3] = invDet * (
        m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
        m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
        m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0])
    );

    return result;
}

Mat4 Mat4::perspective(float fovy, float aspect, float near, float far) {
    Mat4 result(0.0f); // Matrice nulle

    float tanHalfFovy = std::tan(fovy / 2.0f);

    result.m[0][0] = 1.0f / (aspect * tanHalfFovy);
    result.m[1][1] = 1.0f / tanHalfFovy;
    result.m[2][2] = -(far + near) / (far - near);
    result.m[2][3] = -1.0f;
    result.m[3][2] = -(2.0f * far * near) / (far - near);

    return result;
}

Mat4 Mat4::lookAt(float eyeX, float eyeY, float eyeZ,
                  float centerX, float centerY, float centerZ,
                  float upX, float upY, float upZ) {
    float forwardX = centerX - eyeX;
    float forwardY = centerY - eyeY;
    float forwardZ = centerZ - eyeZ;

    float forwardLength = std::sqrt(forwardX * forwardX + forwardY * forwardY + forwardZ * forwardZ);
    if (forwardLength > 0.0f) {
        forwardX /= forwardLength;
        forwardY /= forwardLength;
        forwardZ /= forwardLength;
    }

    float rightX = upY * forwardZ - upZ * forwardY;
    float rightY = upZ * forwardX - upX * forwardZ;
    float rightZ = upX * forwardY - upY * forwardX;

    float rightLength = std::sqrt(rightX * rightX + rightY * rightY + rightZ * rightZ);
    if (rightLength > 0.0f) {
        rightX /= rightLength;
        rightY /= rightLength;
        rightZ /= rightLength;
    }

    float newUpX = forwardY * rightZ - forwardZ * rightY;
    float newUpY = forwardZ * rightX - forwardX * rightZ;
    float newUpZ = forwardX * rightY - forwardY * rightX;

    Mat4 result;

    result.m[0][0] = rightX;
    result.m[0][1] = newUpX;
    result.m[0][2] = -forwardX;
    result.m[0][3] = 0.0f;

    result.m[1][0] = rightY;
    result.m[1][1] = newUpY;
    result.m[1][2] = -forwardY;
    result.m[1][3] = 0.0f;

    result.m[2][0] = rightZ;
    result.m[2][1] = newUpZ;
    result.m[2][2] = -forwardZ;
    result.m[2][3] = 0.0f;

    result.m[3][0] = -(rightX * eyeX + rightY * eyeY + rightZ * eyeZ);
    result.m[3][1] = -(newUpX * eyeX + newUpY * eyeY + newUpZ * eyeZ);
    result.m[3][2] = -(-forwardX * eyeX + -forwardY * eyeY + -forwardZ * eyeZ);
    result.m[3][3] = 1.0f;

    return result;
}

// === Opérateurs externes ===

Mat4 operator*(float scalar, const Mat4& mat) {
    return mat * scalar;
}

std::ostream& operator<<(std::ostream& os, const Mat4& mat) {
    os << std::fixed << std::setprecision(3);
    for (int row = 0; row < 4; ++row) {
        os << "| ";
        for (int col = 0; col < 4; ++col) {
            os << std::setw(8) << mat(col, row) << " ";
        }
        os << "|";
        if (row < 3) os << std::endl;
    }
    return os;
}
