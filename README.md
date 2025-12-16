# game_world

## set up dev env
```bash
sudo apt update
sudo apt install -y build-essential

sudo apt install -y \
  pkg-config \
  libgl1-mesa-dev \
  libx11-dev \
  libxrandr-dev \
  libxi-dev \
  libxcursor-dev \
  libxinerama-dev

sudo apt install -y build-essential ninja-build pkg-config libgl1-mesa-dev libglfw3-dev libglm-dev

```

```bash
mkdir -p external/glad/include external/glad/src
```

```bash
cd \<insert own path here\>/game_world
rm -rf build

cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-linux

cmake --build build -j
```

## Use atlas generator

```bash
./build/atlas_generator /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf

```