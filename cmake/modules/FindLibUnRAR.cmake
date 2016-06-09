# Locate unrar
# This module defines
# LIBUNRAR_LIBRARY
# LIBUNRAR_FOUND, if false, do not try to link to libzip 
# LIBUNRAR_INCLUDE_DIR, where to find the headers
#

FIND_PATH(LIBUNRAR_INCLUDE_DIR dll.hpp
    $ENV{LIBUNRAR_DIR}
    ~/Library/Frameworks/unrar
    /Library/Frameworks/unrar
    /usr/local/include/unrar
    /usr/include/unrar
    /sw/include/unrar # Fink
    /opt/local/include/unrar # DarwinPorts
    /opt/csw/include/unrar # Blastwave
    /opt/include/unrar
    /usr/freeware/include/unrar
)

FIND_LIBRARY(LIBUNRAR_LIBRARY 
    NAMES libunrar unrar
    PATHS
    $ENV{LIBUNRAR_DIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    /usr/freeware/lib64
)

SET(LIBUNRAR_FOUND "NO")
IF(LIBUNRAR_LIBRARY AND LIBUNRAR_INCLUDE_DIR)
	SET(LIBUNRAR_FOUND "YES")
ENDIF(LIBUNRAR_LIBRARY AND LIBUNRAR_INCLUDE_DIR)


