pushd .\..
mkdir build
cd build
Rem call powershell -Command "(Invoke-WebRequest -Uri https://git.io/JnHTY -OutFile install_zlib.bat)"; ./install_zlib.bat; del install_zlib.bat
call cmake -G "Visual Studio 17 2022" ../.
popd
PAUSE