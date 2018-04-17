CXX := -O3 main.cpp makelevelset3.cpp -std=c++11
WASM := -s DEMANGLE_SUPPORT=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "FS_createDataFile", "FS_readFile", "FS_unlink"]' -s ALLOW_MEMORY_GROWTH=1 -s WASM=1 -o js/SDFGen.js -s EXPORTED_FUNCTIONS='["_SDFGen"]'

all:
	g++ ${CXX}
wasm:
	em++ ${CXX} ${WASM}
