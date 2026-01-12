# audio_lib
Library of Audio Related Utilities

Requires the lfmq (my own library), portaudio, sndfile, and fftw3 libraries to be installed onto the system

## Build Instructions:
```
mkdir build
cd build
cmake ..
cmake --build .
```

## Installation Instructions:
```
cd build
cmake --install .
```

## To build tests:
```
cd build
cmake .. -DBUILD_TESTS=ON
cmake --build .
```
