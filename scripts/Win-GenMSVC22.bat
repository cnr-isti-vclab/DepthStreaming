pushd .\..
mkdir build
cd build
call cmake -G "Visual Studio 17 2022" ../.
popd
PAUSE