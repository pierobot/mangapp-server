CMAKE_MINIMUM_REQUIRED(VERSION 3.1.0 FATAL_ERROR)

IF (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()

SET(PROJECT_NAME "mangapp-server")
PROJECT(${PROJECT_NAME} C CXX)
SET(${PROJECT_NAME}_MAJOR_VERSION 0)
SET(${PROJECT_NAME}_MINOR_VERSION 1)
SET(${PROJECT_NAME}_PATCH_VERSION 0)

FILE(GLOB SERVER_SOURCES src/archive.cpp src/archive.hpp
                         src/archive_entry.cpp src/archive_entry.hpp
                         
                         src/archive_7z.cpp src/archive_7z.hpp
                         src/archive_7z_entry.cpp src/archive_7z_entry.hpp
                         
                         src/archive_rar.cpp src/archive_rar.hpp
                         src/archive_rar_entry.cpp src/archive_rar_entry.hpp
                         
                         src/archive_zip.cpp src/archive_zip.hpp
                         src/archive_zip_entry.cpp src/archive_zip_entry.hpp
						                         
                         src/directory_entry.hpp
                         src/directory_file_entry.cpp src/directory_file_entry.hpp
                         src/file_enumeration.hpp
                         
						 src/http_client.cpp src/http_client.hpp
						 src/http_request.cpp src/http_request.hpp
						 src/http_response.cpp src/http_response.hpp
                         src/http_utility.cpp src/http_utility.hpp
                         
                         src/library.hpp
                         src/main.cpp

                         src/manga_directory_entry.cpp src/manga_directory_entry.hpp
                         src/manga_entry.cpp src/manga_entry.hpp
                         src/manga_library.cpp src/manga_library.hpp
                         
                         src/mangaupdates_parser.cpp src/mangaupdates_parser.hpp
                         
                         src/server.cpp src/server.hpp
                         src/sort.hpp

						 src/string.hpp
                         
                         src/users.cpp src/users.hpp
                         src/utf8.cpp src/utf8.hpp
                         
                         src/image.cpp src/image.hpp
)

FILE(GLOB JSON11_SOURCES thirdparty/json11/json11.cpp thirdparty/json11/json11.h)
FILE(GLOB 7Z_SOURCES thirdparty/7z/7zAlloc.c
                     thirdparty/7z/7zBuf.c
                     thirdparty/7z/7zCrc.c
                     thirdparty/7z/7zCrcOpt.c
                     thirdparty/7z/7zFile.c
                     thirdparty/7z/7zDec.c
                     thirdparty/7z/7zArcIn.c
                     thirdparty/7z/7zStream.c
                     thirdparty/7z/Bcj2.c
                     thirdparty/7z/Bra.c
                     thirdparty/7z/Bra86.c
                     thirdparty/7z/BraIA64.c
                     thirdparty/7z/CpuArch.c
                     thirdparty/7z/Delta.c
                     thirdparty/7z/Lzma2Dec.c
                     thirdparty/7z/LzmaDec.c
                     thirdparty/7z/Ppmd7.c
                     thirdparty/7z/Ppmd7Dec.c)

ADD_EXECUTABLE(${PROJECT_NAME} ${7Z_SOURCES} 
                               ${JSON11_SOURCES}
                               ${SERVER_SOURCES})


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} thirdparty)

IF (MSVC)
    SET(CMAKE_USE_WIN32_THREADS_INIT true)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_WARNINGS 
                    -D_CRT_SECURE_NO_WARNINGS 
                    -D_SCL_SECURE_NO_WARNINGS
                    -DWIN32_LEAN_AND_MEAN
                    -D_WIN32_WINNT=0x0601
                    -DCROW_MSVC_WORKAROUND
                    -DZIP_STATIC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4819")
    SET(Boost_USE_STATIC_LIBS ON)
ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
    ADD_DEFINITIONS(-DUSE_BOOST_REGEX)
    SET(Boost_USE_STATIC_LIBS OFF)
ENDIF()

ADD_DEFINITIONS(-DCROW_ENABLE_SSL)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

# --------------------------------------------------------------
SET(Boost_USE_MULTITHREADED ON)
FIND_PACKAGE(Boost COMPONENTS date_time filesystem locale program_options random regex system REQUIRED)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
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
    INCLUDE_DIRECTORIES(${LIBZIP_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIBZIP_LIBRARY})
ELSE()
    MESSAGE(FATAL_ERROR "Unable to locate libzip.")
ENDIF()
# -------------------------------------------------------------
FIND_PACKAGE(LibUnRAR REQUIRED)
IF(LIBUNRAR_FOUND)
    INCLUDE_DIRECTORIES(${LIBUNRAR_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIBUNRAR_LIBRARY})
ELSE()
    MESSAGE(FATAL_ERROR "Unable to locate libunrar.")
ENDIF()
# -------------------------------------------------------------
FIND_PACKAGE(mstch REQUIRED)
IF(mstch_FOUND)
    INCLUDE_DIRECTORIES(${mstch_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} mstch::mstch)
ELSE()
    MESSAGE(FATAL_ERROR "Unable to locate mstch.")
ENDIF()
#--------------------------------------------------------------
FIND_PACKAGE(OpenSSL REQUIRED)
IF(OPENSSL_FOUND)
    INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
ELSE()
    MESSAGE(FATAL_ERROR "Unable to locate OpenSSL.")
ENDIF()
#--------------------------------------------------------------
FIND_PACKAGE(OpenCV REQUIRED core imgproc imgcodecs videoio highgui)
IF(OPENCV_CORE_FOUND AND OPENCV_IMGPROC_FOUND AND OPENCV_IMGCODECS_FOUND AND
   OPENCV_VIDEOIO_FOUND AND OPENCV_HIGHGUI_FOUND)
    INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${OpenCV_LIBRARIES})
ELSE()
    MESSAGE(FATAL_ERROR "Unable to locate opencv libraries.")
ENDIF()

TARGET_LINK_LIBRARIES(${PROJECT_NAME})