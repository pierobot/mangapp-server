# Locate unrar
# This module defines
# LIBUNRAR_LIBRARY
# LIBUNRAR_FOUND, if false, do not try to link to libzip 
# LIBUNRAR_INCLUDE_DIR, where to find the headers
#

FIND_PATH(LIBUNRAR_INCLUDE_DIR rar.hpp dll.hpp
    $ENV{LIBUNRAR_DIR}/include
    $ENV{LIBUNRAR_DIR}
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
    /usr/freeware/include
)

FIND_LIBRARY(LIBUNRAR_LIBRARY 
    NAMES libunrar unrar
    PATHS
    $ENV{LIBUNRAR_DIR}/lib
    $ENV{LIBUNRAR_DIR}
    $ENV{OSGDIR}/lib
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/lib
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
    /usr/freeware/lib64
)

SET(LIBUNRAR_FOUND "NO")
IF(LIBUNRAR_LIBRARY AND LIBUNRAR_INCLUDE_DIR)
	SET(LIBUNRAR_FOUND "YES")
ENDIF(LIBUNRAR_LIBRARY AND LIBUNRAR_INCLUDE_DIR)


