dans le dossiers ./models
il y a tjrs un fichier .obj avec son .mtl

Dans le main.cpp on va charger le modèle 3D et l'afficher à l'écran.

- On va utiliser la bibliothèque TinyOBJLoader pour charger le modèle et ses textures.
- On va aussi utiliser GLFW pour créer une fenêtre et gérer les entrées utilisateur.

Pour lancer le programme, il faut d'abord compiler le code.

Avec MINGW64 :

```bash

pacman -S mingw-w64-x86_64-toolchain \
           mingw-w64-x86_64-glew \
           mingw-w64-x86_64-glfw \
           mingw-w64-x86_64-glm \
           mingw-w64-x86_64-openal \
           mingw-w64-x86_64-cmake

cd ..
rm -rf build        # supprime l’ancienne build (cache CMake, binaires…)
mkdir build && cd build
cmake -G "Unix Makefiles" ..
make
./ProjetOpenGL.exe
```

Puis exécuter le binaire :

```bash
./ProjetOpenGL.exe
```
