#!/bin/bash

cd tools
rake cmake
rake make
cd ..
native-Build/bin/Moss
