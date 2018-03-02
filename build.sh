rm CMakeCache.txt
rm -rf vstsid.vst
rm -rf build
mkdir build
cd build
cmake ..
make
