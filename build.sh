SOURCES=$(find src/*.cpp)
INCLUDE_DIRS='include/'

mkdir -p 'bin/'

g++ -std=c++11 -framework opencl -I$INCLUDE_DIRS -o bin/Assn3 $SOURCES