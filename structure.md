/ProjetOpenGL
│
├── /include
│ ├── Camera.h // Classe pour gérer la caméra 3D
│ ├── Shader.h // Classe utilitaire pour charger et compiler les shaders
│ ├── Mesh.h // Classe pour représenter un objet 3D chargé depuis TinyOBJLoader
│ ├── Model.h // Classe pour charger un modèle .obj complet (via TinyOBJLoader)
│ ├── Transform.h // Classe pour la gestion Translation / Rotation / Scale
│ └── Mat4.h // (Optionnel) propre classe mat4 (multiplication de matrices)
│
├── /src
│ ├── main.cpp // Fichier principal : initialisation OpenGL, boucle principale
│ ├── Camera.cpp
│ ├── Shader.cpp
│ ├── Mesh.cpp
│ ├── Model.cpp
│ ├── Transform.cpp
│ └── Mat4.cpp // (Optionnel) Implémentation de la classe mat4
│
├── /shaders
│ ├── simple_color.vert // Shader de vertex simple (couleur simple)
│ ├── simple_color.frag // Shader de fragment simple (couleur simple)
│ ├── textured.vert // Shader de vertex pour objets texturés
│ ├── textured.frag // Shader de fragment pour objets texturés
│ ├── envmap.vert // Shader vertex pour environment mapping
│ ├── envmap.frag // Shader fragment pour environment mapping
│ └── phong.frag // Shader de fragment pour Phong/Blinn-Phong Lighting
│
├── /models
│ ├── objet1.obj // Modèle 3D pour un objet (exemple)
│ ├── objet2.obj
│ └── ...
│
├── /textures
│ ├── texture1.png // Textures utilisées pour certains objets
│ ├── texture2.jpg
│ └── ...
│
├── /libs
│ ├── tiny_obj_loader.h // TinyOBJLoader (copier ici le .h et le .cc)
│ ├── tiny_obj_loader.cc
│ └── imgui/ // (Optionnel) si on veut ImGui pour GUI
│
├── /FBO
│ ├── FBO.h // (Optionnel) Gestion des Framebuffer Objects
│ └── FBO.cpp
│
├── README.md // (Optionnel) Petit fichier pour expliquer l'organisation et le build
└── ressources/ // (Optionnel) Fichiers annexes comme le logo du projet, autres assets et le sujet de projet
