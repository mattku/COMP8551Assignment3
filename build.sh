SOURCES=$(find src/*.cpp)
INCLUDE_DIRS='include/'

#
#OpenCL library path goes here.
LINUX_OPENCL_PATH=""
#
#
mkdir -p 'bin/'

case "$(uname -s)" in

   Darwin)
     echo 'Mac OS X'
     g++ -std=c++11 -framework opencl -I$INCLUDE_DIRS -o bin/Assn3 $SOURCES
     ;;

   Linux)
     echo 'Linux'
     g++ -std=c++11 -I$INCLUDE_DIRS -o bin/Assn3 $SOURCES -L$LINUX_OPENCL_PATH -lOpenCL 
     ;;

   CYGWIN*|MINGW32*|MSYS*|MINGW64*)
     echo 'WINDOWS IS NOT SUPPORTED'
     exit 1
     ;;

   *)
     echo 'Unknown OS???' 
     exit 1
     ;;
esac
echo 'Build Completed'