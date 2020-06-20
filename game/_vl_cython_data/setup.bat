echo off
cls
python _setup.py build_ext --build-lib "modules" --build-temp "temp"
pause
