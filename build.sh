#!/bin/bash

clang++ -std=c++20 -shared -fPIC -o rct.so bindings.cc rct.cc random.cc
