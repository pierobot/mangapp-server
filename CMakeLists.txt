CMAKE_MINIMUM_REQUIRED(VERSION 3.1.0 FATAL_ERROR)

SET(${CMAKE_BUILD_TYPE} Debug)

SET(PROJECT_NAME "mangapp-server")
PROJECT(${PROJECT_NAME} C CXX)
SET(${PROJECT_NAME}_MAJOR_VERSION 0)
SET(${PROJECT_NAME}_MINOR_VERSION 1)
SET(${PROJECT_NAME}_PATCH_VERSION 0)

FILE(GLOB SERVER_SOURCES src/*.cpp src/*.hpp)
FILE(GLOB JSON11_SOURCES include/json11/json11.cpp include/json11/json11.h)
FILE(GLOB 7Z_SOURCES include/7z/7zAlloc.c
                     include/7z/7zBuf.c
                     include/7z/7zCrc.c
                     include/7z/7zCrcOpt.c
                     include/7z/7zFile.c
                     include/7z/7zDec.c
                     include/7z/7zArcIn.c
                     include/7z/7zStream.c
                     include/7z/Bcj2.c
                     include/7z/Bra.c
                     include/7z/Bra86.c
                     include/7z/BraIA64.c
                     include/7z/CpuArch.c
                     include/7z/Delta.c
                     include/7z/Lzma2Dec.c
                     include/7z/LzmaDec.c
                     include/7z/Ppmd7.c
                     include/7z/Ppmd7Dec.c)

ADD_EXECUTABLE(${PROJECT_NAME} ${7Z_SOURCES} 
                               ${JSON11_SOURCES}
                               ${SERVER_SOURCES})


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} include)

IF (WIN32)
    SET(CMAKE_USE_WIN32_THREADS_INIT true)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_WARNINGS 
                    -D_CRT_SECURE_NO_WARNINGS 
                    -D_SCL_SECURE_NO_WARNINGS) 
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
ENDIF()

#TARGET_COMPILE_OPTIONS(${PROJECT_NAME} PUBLIC -std=c++11 -Wall)


LIST(APPEND CMAKE_MODULE_PATH "cmake/modules")

# --------------------------------------------------------------
SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
FIND_PACKAGE(Boost COMPONENTS locale filesystem system REQUIRED)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${Boost_LIBRARIES})
ELSE()
    MESSAGE(FATAL_ERROR "Unable to locate boost.")
ENDIF()
# ---------------------------------------------------------------
FIND_PACKAGE(Threads REQUIRED)
IF (Threads_FOUND)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
ELSE()
    MESSAGE(FATAL_ERROR "Unable to locate thread library.")
ENDIF()
# -------------------------------------------------------------
FIND_PACKAGE(ZLIB REQUIRED)
IF(ZLIB_FOUND)
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${ZLIB_LIBRARIES})
ELSE()
    MESSAGE(FATAL_ERROR "Unable to locate zlib.")
ENDIF()
# -------------------------------------------------------------
FIND_PACKAGE(LibZip REQUIRED)
IF(LIBZIP_FOUND)
    MESSAGE(STATUS ${LIBZIP_LIBRARY})
    INCLUDE_DIRECTORIES(${LIBZIP_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIBZIP_LIBRARY})
ELSE()
    MESSAGE(FATAL_ERROR "Unable to locate libzip.")
ENDIF()

TARGET_LINK_LIBRARIES(${PROJECT_NAME})
# -------------------------------------------------------------
FIND_PACKAGE(LibUnRAR REQUIRED)
IF(LIBUNRAR_FOUND)
    INCLUDE_DIRECTORIES(${LIBUNRAR_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIBUNRAR_LIBRARY})
ELSE()
    MESSAGE(FATAL_ERROR "Unable to locate libunrar.")
ENDIF()
