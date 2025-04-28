# Projet OpenGL – ESIEE IT Computer Graphics

🔮 Projet de Computer Graphics @ESIEE Paris

## Objectif
Créer une scène 3D en utilisant OpenGL (2.x ou 3.x) avec plusieurs objets et permettre à l'utilisateur de naviguer dans la scène.

## Fonctionnalités attendues

### Partie 1 : Affichage
- Affichage de plusieurs objets différents, chacun utilisant différents shaders :
  - Un objet avec une couleur simple.
  - Un objet avec une texture.
  - Un objet utilisant une environment map.
- Prise en compte correcte du **sRGB**.
- Chargement d'objets 3D au format **OBJ** avec la bibliothèque **TinyOBJLoader** :
  - Gestion des matériaux (couleurs ambiantes, diffuses, spéculaires).
- Mise en place d'un système d'illumination :
  - Phong ou Blinn-Phong.
  - Possibilité d'appliquer différents modèles d'éclairage selon les objets (Lambert, Phong...).

### Partie 2 : Navigation
- Placement des objets à différentes positions, avec des translations, rotations et échelles différentes.
- Déplacement libre dans la scène ou orbite autour d'objets via une caméra 3D :
  - Utilisation possible de **UBO** pour la gestion de la projection, de la caméra et des transformations.

### Partie 3 : Options supplémentaires (bonus)
- Implémentation d'une classe `mat4` en C++ incluant la multiplication de matrices.
- Passage de la matrice World directement en uniform (ou via UBO) plutôt que les matrices séparées.
- Utilisation de **Framebuffer Objects (FBO)** pour du post-traitement graphique.
- Intégration d'une **interface graphique** avec **IMGUI** pour interagir avec la scène.

## Librairies utilisées
- [TinyOBJLoader](https://github.com/tinyobjloader/tinyobjloader) — Chargement de modèles 3D `.obj`
- [IMGUI](https://github.com/ocornut/imgui) — Interface graphique immédiate pour C++

## Pré-requis
- OpenGL 2.x ou 3.x
- C++
- GLFW ou SDL (pour la gestion de la fenêtre et du contexte OpenGL)
- GLAD ou GLEW (pour le chargement des extensions OpenGL)

## Instructions de compilation
1. Cloner le projet :
   ```bash
   git clone https://github.com/Berachem/Computer_Graphics.git
   cd .\Computer_Graphics\
   ```
2. Compiler le projet :
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```
3. Exécuter :
   ```bash
   ./nom_du_binaire
   ```

## Structure du projet (exemple)
```
/src
    main.cpp
    renderer/
        shader.cpp
        camera.cpp
        mesh.cpp
    utils/
        mat4.cpp
    ...
/assets
    models/
    textures/
CMakeLists.txt
README.md
```

## Liens utiles
- [TinyOBJLoader - README et exemples](https://github.com/tinyobjloader/tinyobjloader)
- [IMGUI - Documentation](https://github.com/ocornut/imgui)
- [Tutoriel OpenGL moderne](https://learnopengl.com/)

## Contact
Pour toute question : [malek.bengougam@esiee.fr](mailto:malek.bengougam@esiee.fr)

