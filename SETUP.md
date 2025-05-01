dans le dossiers ./models
il y a tjrs un fichier .obj avec son .mtl

Dans le main.cpp on va charger le modèle 3D et l'afficher à l'écran.

- On va utiliser la bibliothèque TinyOBJLoader pour charger le modèle et ses textures.
- On va aussi utiliser GLFW pour créer une fenêtre et gérer les entrées utilisateur.

Pour lancer le programme, il faut d'abord compiler le code.

Avec MINGW64 :

```bash
cd ./build
cmake ..
make
```

Puis exécuter le binaire :

```bash
./ProjetOpenGL.exe
```
