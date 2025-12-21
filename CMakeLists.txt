cmake_minimum_required(VERSION 3.10)
project(UkrCoop)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -fPIC")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,notext")

# SourceMod and SDKs
set(SMSDK ../../sourcemod)
set(HL2SDK ../../hl2sdk-l4d)
set(MMSOURCE110 ../../mmsource)
set(LUA54 ../../lua54)
set(BREAKPAD ../../break_pad)

include(src/source.cmake)

# Include directories
include_directories(
    .
    ..
    include
    include/sdk
    include/GeoIP_SDK
    include/includ_lua
    include/Interface
    include/LuaBridge
    LuaBridge
    ${HL2SDK}/game/shared
    ${HL2SDK}/game/server
    ${HL2SDK}/common
    ${HL2SDK}/public
    ${HL2SDK}/public/game/server
    ${HL2SDK}/public/toolframework
    ${HL2SDK}/public/tier0
    ${HL2SDK}/public/tier1
    ${HL2SDK}/public/engine
    ${HL2SDK}/public/mathlib
    ${HL2SDK}/public/appframework
    ${SMSDK}/public
    ${SMSDK}/public/core
    ${SMSDK}/public/sourcepawn
    ${MMSOURCE110}/core
    ${MMSOURCE110}/core/sourcehook
    ${LUA54}/include
    ${BREAKPAD}
)

# Compiler definitions
add_definitions(
    -DSOURCE_ENGINE=8 -DL4D1
    -DSE_EPISODEONE=1 -DSE_DARKMESSIAH=2 -DSE_ORANGEBOX=3
    -DSE_BLOODYGOODTIME=4 -DSE_EYE=5 -DSE_CSS=6 -DSE_ORANGEBOXVALVE=7
    -DSE_LEFT4DEAD=8 -DSE_LEFT4DEAD2=9 -DSE_ALIENSWARM=10
    -DSE_PORTAL2=11 -DSE_CSGO=12
    -DPOSIX -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp
    -Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf
    -D_alloca=alloca -Dstrcmpi=strcasecmp -DCOMPILER_GCC -DSOURCEMOD_BUILD
    -DHAVE_STDINT_H -DNDEBUG -D_LINUX
)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-class-memaccess -Wno-overloaded-virtual -Wno-switch -Wno-unused -Wno-non-virtual-dtor -Wno-delete-non-virtual-dtor -msse -O3 -funroll-loops -pipe -fno-strict-aliasing -fno-inline-functions -mfpmath=sse -fvisibility=hidden -fno-exceptions -fvisibility-inlines-hidden -fno-devirtualize")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -Werror -Wno-switch -Wno-unused -msse -O3 -funroll-loops -pipe -fno-strict-aliasing -fno-inline-functions -mfpmath=sse -fvisibility=hidden -fno-exceptions -Wno-unused-result")
# Static libraries
set(STATIC_LIBS
    /home/denmarko/projects/hl2sdk-l4d/lib/linux/tier1_i486.a
    /home/denmarko/projects/hl2sdk-l4d/lib/linux/mathlib_i486.a
    /home/denmarko/projects/lua54/lib32/liblua.a
)

# Shared libraries
set(SHARED_LIBS
    ${CMAKE_SOURCE_DIR}/src/libvstdlib.so
    ${CMAKE_SOURCE_DIR}/src/libtier0.so
)

# Output
add_library(UkrCoop SHARED ${SOURCES})
target_link_libraries(UkrCoop ${SHARED_LIBS} ${STATIC_LIBS} dl stdc++)

# Output directory
set_target_properties(UkrCoop PROPERTIES
    PREFIX ""
    OUTPUT_NAME "UkrCoop.ext"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Release"
)

