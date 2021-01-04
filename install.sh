#!/bin/sh

# Luodaan asennuskansio OGRElle ja Spaceballsin build kansio
mkdir -p dependencies/OGRE
mkdir build

# Ladataan OGRE
git submodule init
git submodule update

# Buildataan OGRE ja asennetaan dependencies/OGRE kansioon
mkdir submodules/ogre/build
cd submodules/ogre/build

cmake .. \
    -DOGRE_BUILD_PLUGIN_ASSIMP=FALSE \
    -DOGRE_BUILD_PLUGIN_EXRCODEC=FALSE \
    -DOGRE_BUILD_PLUGIN_FREEIMAGE=FALSE \
    -DOGRE_BUILD_PLUGIN_DOT_SCENE=FALSE \
    -DOGRE_BUILD_PLUGIN_OCTREE=FALSE \
    -DOGRE_BUILD_PLUGIN_BSP=FALSE \
    -DOGRE_BUILD_PLUGIN_CG=FALSE \
    -DOGRE_BUILD_PLUGIN_PCZ=FALSE \
    -DOGRE_BUILD_PLUGIN_PFX=FALSE

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
	cmake --build .
	cmake --install . --prefix ../../../dependencies/OGRE
else
	cmake --build . --config release --target INSTALL
	cp -rf sdk/* ../../../dependencies/OGRE/.
    cp ../../../dependencies/OGRE/lib/*.dll ../../../build
fi
	
# Luodaan Spaceballsin build-tiedostot
cd ../../..
cmake -S . -B build
