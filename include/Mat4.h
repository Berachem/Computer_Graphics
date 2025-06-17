#ifndef MAT4_H
#define MAT4_H

#include <array>
#include <iostream>
#include <cmath>

/**
 * @brief Classe Mat4 - Matrice 4x4 pour les transformations 3D
 * 
 * Cette classe implémente une matrice 4x4 avec les opérations essentielles
 * pour les transformations 3D en OpenGL. Elle utilise un stockage en colonne
 * majeure (column-major) pour être compatible avec OpenGL.
 */
class Mat4 {
private:
    // Stockage en colonne majeure (column-major) comme OpenGL
    // m[colonne][ligne]
    std::array<std::array<float, 4>, 4> m;

public:
    // === Constructeurs ===
    
    /**
     * @brief Constructeur par défaut - Crée une matrice identité
     */
    Mat4();
    
    /**
     * @brief Constructeur avec valeur diagonale
     * @param diagonal Valeur à placer sur la diagonale
     */
    explicit Mat4(float diagonal);
    
    /**
     * @brief Constructeur avec toutes les valeurs
     * @param values Tableau de 16 valeurs en ordre column-major
     */
    explicit Mat4(const std::array<float, 16>& values);
    
    /**
     * @brief Constructeur de copie
     */
    Mat4(const Mat4& other) = default;
    
    /**
     * @brief Opérateur d'assignation
     */
    Mat4& operator=(const Mat4& other) = default;

    // === Accès aux éléments ===
    
    /**
     * @brief Accès à un élément (lecture/écriture)
     * @param col Colonne (0-3)
     * @param row Ligne (0-3)
     * @return Référence à l'élément
     */
    float& operator()(int col, int row);
    
    /**
     * @brief Accès à un élément (lecture seule)
     * @param col Colonne (0-3)
     * @param row Ligne (0-3)
     * @return Valeur de l'élément
     */
    const float& operator()(int col, int row) const;
    
    /**
     * @brief Accès à une colonne complète
     * @param col Index de la colonne (0-3)
     * @return Référence au tableau de la colonne
     */
    std::array<float, 4>& operator[](int col);
    
    /**
     * @brief Accès à une colonne complète (lecture seule)
     * @param col Index de la colonne (0-3)
     * @return Référence constante au tableau de la colonne
     */
    const std::array<float, 4>& operator[](int col) const;

    // === Opérations matricielles ===
    
    /**
     * @brief Multiplication de matrices
     * @param other Matrice à multiplier
     * @return Résultat de la multiplication
     */
    Mat4 operator*(const Mat4& other) const;
    
    /**
     * @brief Multiplication par un scalaire
     * @param scalar Scalaire à multiplier
     * @return Résultat de la multiplication
     */
    Mat4 operator*(float scalar) const;
    
    /**
     * @brief Addition de matrices
     * @param other Matrice à additionner
     * @return Résultat de l'addition
     */
    Mat4 operator+(const Mat4& other) const;
    
    /**
     * @brief Soustraction de matrices
     * @param other Matrice à soustraire
     * @return Résultat de la soustraction
     */
    Mat4 operator-(const Mat4& other) const;

    // === Méthodes utilitaires ===
    
    /**
     * @brief Transpose la matrice
     * @return Matrice transposée
     */
    Mat4 transpose() const;
    
    /**
     * @brief Calcule le déterminant de la matrice
     * @return Déterminant
     */
    float determinant() const;
    
    /**
     * @brief Calcule l'inverse de la matrice
     * @return Matrice inverse (ou matrice identité si non inversible)
     */
    Mat4 inverse() const;
    
    /**
     * @brief Retourne un pointeur vers les données (pour OpenGL)
     * @return Pointeur vers les données en format column-major
     */
    const float* data() const;
    
    /**
     * @brief Affiche la matrice (pour debug)
     */
    void print() const;

    // === Méthodes statiques pour les transformations ===
    
    /**
     * @brief Crée une matrice identité
     * @return Matrice identité
     */
    static Mat4 identity();
    
    /**
     * @brief Crée une matrice de translation
     * @param x Translation en X
     * @param y Translation en Y
     * @param z Translation en Z
     * @return Matrice de translation
     */
    static Mat4 translate(float x, float y, float z);
    
    /**
     * @brief Crée une matrice de rotation autour de l'axe X
     * @param angle Angle en radians
     * @return Matrice de rotation
     */
    static Mat4 rotateX(float angle);
    
    /**
     * @brief Crée une matrice de rotation autour de l'axe Y
     * @param angle Angle en radians
     * @return Matrice de rotation
     */
    static Mat4 rotateY(float angle);
    
    /**
     * @brief Crée une matrice de rotation autour de l'axe Z
     * @param angle Angle en radians
     * @return Matrice de rotation
     */
    static Mat4 rotateZ(float angle);
    
    /**
     * @brief Crée une matrice de mise à l'échelle
     * @param x Facteur d'échelle en X
     * @param y Facteur d'échelle en Y
     * @param z Facteur d'échelle en Z
     * @return Matrice de mise à l'échelle
     */
    static Mat4 scale(float x, float y, float z);
    
    /**
     * @brief Crée une matrice de projection perspective
     * @param fovy Champ de vision vertical en radians
     * @param aspect Ratio d'aspect (largeur/hauteur)
     * @param near Distance du plan proche
     * @param far Distance du plan lointain
     * @return Matrice de projection perspective
     */
    static Mat4 perspective(float fovy, float aspect, float near, float far);
    
    /**
     * @brief Crée une matrice de vue (lookAt)
     * @param eyeX Position X de l'œil
     * @param eyeY Position Y de l'œil
     * @param eyeZ Position Z de l'œil
     * @param centerX Position X du centre regardé
     * @param centerY Position Y du centre regardé
     * @param centerZ Position Z du centre regardé
     * @param upX Composante X du vecteur up
     * @param upY Composante Y du vecteur up
     * @param upZ Composante Z du vecteur up
     * @return Matrice de vue
     */
    static Mat4 lookAt(float eyeX, float eyeY, float eyeZ,
                       float centerX, float centerY, float centerZ,
                       float upX, float upY, float upZ);
};

// === Opérateurs externes ===

/**
 * @brief Multiplication scalaire à gauche
 * @param scalar Scalaire
 * @param mat Matrice
 * @return Résultat de la multiplication
 */
Mat4 operator*(float scalar, const Mat4& mat);

/**
 * @brief Opérateur de sortie pour l'affichage
 * @param os Stream de sortie
 * @param mat Matrice à afficher
 * @return Stream de sortie
 */
std::ostream& operator<<(std::ostream& os, const Mat4& mat);

#endif // MAT4_H
