# HelloOpenGL

### Building libglfw.dylib on Mac OS X

```c
git clone https://github.com/glfw/glfw.git
cd glfw
mkdir build
cd build
export MACOSX_DEPLOYMENT_TARGET=10.12
cmake -D GLFW_NATIVE_API=1 -D CMAKE_OSX_ARCHITECTURES="i386;x86_64" -D BUILD_SHARED_LIBS=ON -D CMAKE_C_COMPILER=clang ../
make
make install
```
