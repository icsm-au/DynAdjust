#
# Find the native Xerces includes and library
#
# XERCESC_INCLUDE_DIR - where to find dom/dom.hpp, etc.
# XERCESC_LIBRARY     - List of fully qualified libraries to link against when using Xerces.
# XERCESC_FOUND       - Do not attempt to use Xerces if "no" or undefined.
#
# Look for installation handled by package manager, then manual installation folder
# - Fedora installation:
#     /usr/lib64
#     /usr/include/xercesc
# - Ubuntu installation:
#     /usr/lib64 (10.04)
#     /usr/lib/x86_64-linux-gnu (12.04)
#     /usr/include/xercesc
# - Manual installation (download, make, install):
#     /opt/xerces-c/3.1.4

SET (PKG_MGR_PATH_LIB /usr/lib)
SET (PKG_MGR_PATH_INCLUDE /usr/include)

# SET THE ROOT DIRECTORY WHERE XERCES-C++ IS INSTALLED
IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
  # Apple
  SET (XERCESC_ROOT_DIR /usr/local/Cellar/xerces-c/3.2.2)
  SET (XERCESC_LIBRARY_DIR ${XERCESC_ROOT_DIR}/lib)
  SET (XERCESC_INCLUDE_DIR ${XERCESC_ROOT_DIR}/include)

ELSEIF (UNIX)
  SET(XERCESC_ROOT_DIR ${PKG_MGR_PATH_INCLUDE}/xercesc)
  # Various options...
  IF (EXISTS ${PKG_MGR_PATH_LIB}/x86_64-linux-gnu)
    # Modern Ubuntu
    SET (XERCESC_LIBRARY_DIR /usr/lib/x86_64-linux-gnu)
    SET (XERCESC_INCLUDE_DIR ${PKG_MGR_PATH_INCLUDE}/xercesc)
  ELSEIF (EXISTS ${PKG_MGR_PATH_LIB}64/xercesc)
    # Fedora, RHEL, CentOS and early Ubuntu
    SET (XERCESC_LIBRARY_DIR ${PKG_MGR_PATH_LIB}64)
    SET (XERCESC_INCLUDE_DIR ${PKG_MGR_PATH_INCLUDE}/xercesc)
  ELSE ()
    # Manual installation
    SET (XERCESC_ROOT_DIR /opt/xerces-c/3.1.4)
    SET (XERCESC_LIBRARY_DIR ${XERCESC_ROOT_DIR}/lib)
    SET (XERCESC_INCLUDE_DIR ${XERCESC_ROOT_DIR}/include)
  ENDIF ()

ELSE ()
  # Windows
  SET (XERCESC_ROOT_DIR "C:/Data/xerces-c/3.1.4")
  SET (XERCESC_LIBRARY_DIR ${XERCESC_ROOT_DIR}/lib)
  SET (XERCESC_INCLUDE_DIR ${XERCESC_ROOT_DIR}/include)
ENDIF ()


FIND_PATH (XERCESC_INCLUDE_DIR dom/DOM.hpp
  PATH_SUFFIXES 
    xerces xerces-c xercesc
  PATHS 
    ${XERCESC_INCLUDE_DIR}
)

FIND_LIBRARY (XERCESC_LIBRARY
  NAMES
    xerces-c
  HINTS
    ${XERCESC_LIBRARY_DIR}
)

message (STATUS "xerces-c include directory is: ${XERCESC_INCLUDE_DIR}")
message (STATUS "xerces-c lib directory is:     ${XERCESC_LIBRARY_DIR}")

IF (EXISTS ${XERCESC_INCLUDE_DIR})
  IF (EXISTS ${XERCESC_LIBRARY_DIR})
    SET (XERCESC_FOUND TRUE )
  ENDIF ()
ENDIF ()

mark_as_advanced(XERCESC_LIBRARY)
