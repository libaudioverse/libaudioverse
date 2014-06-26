mkdir build
cd build
cmake -G "NMake Makefiles" ..
nmake
cd ..
python scripts\build_bindings.py
