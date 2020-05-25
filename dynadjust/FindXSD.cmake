#
# Find the native XSD includes and library
#
# XSD_INCLUDE_DIR - where to find /xsd/cxx/config.hxx, etc.
# XSD_FOUND       - Do not attempt to use XSD if "no" or undefined.

# Look for installation handled by package manager, alternatively, manual installation
# - Fedora installation:
#     /usr/include/xsd
# - Ubuntu installation:
#     /usr/include/xsd
# - Manual installation:
#     /opt/xsd/xsd-4.0.0-x86_64-linux-gnu/libxsd/xsd

SET (PKG_MGR_PATH_INCLUDE /usr/include)

# SET THE ROOT DIRECTORY WHERE XSD++ IS INSTALLED
IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
  # Apple
  SET(XSD_INCLUDE_DIR /usr/local/Cellar/xsd/4.0.0_1/include)

ELSEIF (UNIX)
  
  #SET(XSD_INCLUDE_DIR ${PKG_MGR_PATH_INCLUDE})
  # Various options...
  IF (EXISTS ${PKG_MGR_PATH_INCLUDE}/xsd)
    # Fedora, RHEL, CentOS, early Ubuntu and modern Ubuntu
    SET (XSD_INCLUDE_DIR ${PKG_MGR_PATH_INCLUDE})
  ELSE ()
    # Manual installation
    SET (XSD_INCLUDE_DIR /opt/xsd/xsd-4.0.0-x86_64-linux-gnu/libxsd)
  ENDIF ()

ELSE ()
  # Windows
  SET (XSD_INCLUDE_DIR "C:/Program Files (x86)/CodeSynthesis XSD 4.0/include")
ENDIF ()  

FIND_PATH(XSD_INCLUDE_DIR cxx/config.hxx
    PATH_SUFFIXES 
      xsd
    PATHS 
      ${XSD_INCLUDE_DIR}
)

message (STATUS "xsd include directory is: ${XSD_INCLUDE_DIR}/xsd")

IF (EXISTS ${XSD_INCLUDE_DIR})
    SET (XSD_FOUND TRUE )
ENDIF ()
