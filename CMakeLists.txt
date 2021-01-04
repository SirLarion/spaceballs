cmake_minimum_required(VERSION 3.8.1)
project(spaceballs VERSION 0.1.0)

add_definitions(-DPROJECT_PATH="${CMAKE_SOURCE_DIR}")

find_package(OGRE 1.11
    REQUIRED COMPONENTS Bites RTShaderSystem 
    PATHS ${CMAKE_SOURCE_DIR}/dependencies/OGRE/
)

add_library(game
    game/gamesession.cpp
    game/gamesession.h
    game/ball.cpp
    game/ball.h
    game/player.h
    game/player.cpp
    game/gui.h
    game/gui.cpp
)

link_directories(${CMAKE_BINARY_DIR})

include_directories("${CMAKE_SOURCE_DIR}/sounds/include")
link_directories("${CMAKE_SOURCE_DIR}/sounds/lib")

if(UNIX)
    target_link_libraries(game -L/${CMAKE_SOURCE_DIR}/sounds/linux/ pthread)
    file(COPY ${CMAKE_SOURCE_DIR}/sounds/linux/ DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY ${CMAKE_SOURCE_DIR}/config/linux/ DESTINATION ${CMAKE_BINARY_DIR})
else()
    file(GLOB IRRKLANG_DLLS
	"${CMAKE_SOURCE_DIR}/sounds/*.dll"
    )
    file(COPY ${IRRKLANG_DLLS} DESTINATION ${CMAKE_BINARY_DIR})
    file(COPY ${CMAKE_SOURCE_DIR}/config/windows/ DESTINATION ${CMAKE_BINARY_DIR})
endif()

target_link_libraries(game IrrKlang)
target_link_libraries(game OgreBites OgreRTShaderSystem OgreOverlay)

add_executable(spaceballs main.cpp controller.h)

target_link_libraries(spaceballs game)
