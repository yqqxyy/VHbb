# assumes we are in the build folder
setupATLAS
lsetup asetup
release=`cat ../source/CxAODBootstrap_VHbb/bootstrap/release.txt`
echo "release=$release"
asetup AnalysisBase,$release,here
cp CMakeLists.txt ../source
cmake ../source
cmake --build .
source x86_64-slc6-gcc62-opt/setup.sh
