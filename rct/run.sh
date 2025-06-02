#!/bin/bash

clang++ -std=c++20 -shared -fPIC -o bindings.so bindings.cc rct.cc random.cc
python3 ./bindings.py