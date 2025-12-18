{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "raylib-wasm-shell";

  buildInputs = [
    pkgs.emscripten
    pkgs.raylib
    pkgs.nodejs
    pkgs.python3
    pkgs.zip
    pkgs.bear
  ];

  shellHook = ''
    export EM_CACHE=$PWD/.emcache
    mkdir -p "$EM_CACHE"

    export RAYLIB_INCLUDE_PATH=$PWD/libs/raylib-5.5_webassembly/include
    export RAYLIB_LIB_PATH=$PWD/libs/raylib-5.5_webassembly/lib

    if [ ! -d libs/raylib-5.5_webassembly ]; then
        echo "Downloading raylib WASM build..."
        curl -L \
          https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_webassembly.zip \
          -o raylib-web.zip
        mkdir -p libs
        unzip raylib-web.zip -d libs
        rm raylib-web.zip
        echo "raylib WASM installed in: libs/raylib-5.5_webassembly/"
    fi
    echo "Emscripten ready. Run: make"
  '';
}
