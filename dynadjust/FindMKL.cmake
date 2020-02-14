# a simple cmake script to locate Intel Math Kernel Library

# This script looks for two places:
#	- the environment variable MKLROOT
#	- the directory /opt/intel/mkl


# Stage 1: find the root directory

set(MKLROOT_PATH $ENV{MKLROOT})
set(ICCROOT_PATH "$ENV{ICCROOT}")

if (UNIX)

	if (NOT MKLROOT_PATH)
		# try to find at /opt/intel/mkl
		
		if (EXISTS "/opt/intel/mkl")
			set(MKLROOT_PATH "/opt/intel/mkl")
		endif (EXISTS "/opt/intel/mkl")
	endif (NOT MKLROOT_PATH)

	if (NOT ICCROOT_PATH)
		# try to find at /opt/intel/lib
		
		if (EXISTS "/opt/intel")
			set(ICCROOT_PATH "/opt/intel")
		endif (EXISTS "/opt/intel")
	endif (NOT ICCROOT_PATH)
else ()
  # Windows
  
  set(MKLROOT_PATH "C:/Program Files (x86)/IntelSWTools/compilers_and_libraries_2019.2.190/windows/mkl")

endif ()

# Stage 2: find include path and libraries
if (ICCROOT_PATH)
	if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	    if (CMAKE_SIZEOF_VOID_P MATCHES 8)
	        set(EXPECT_ICC_LIBPATH "${ICCROOT_PATH}/lib/intel64")
	    else (CMAKE_SIZEOF_VOID_P MATCHES 8)
	        set(EXPECT_ICC_LIBPATH "${ICCROOT_PATH}/lib/ia32")
	    endif (CMAKE_SIZEOF_VOID_P MATCHES 8)
	endif (CMAKE_SYSTEM_NAME MATCHES "Linux")
endif (ICCROOT_PATH)

message(STATUS "CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")
message(STATUS "MKLROOT_PATH ${MKLROOT_PATH}")
message(STATUS "ICCROOT_PATH ${ICCROOT_PATH}")
message(STATUS "EXPECT_ICC_LIBPATH ${EXPECT_ICC_LIBPATH}")
	
if (MKLROOT_PATH)
	# root-path found
	
	if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	    if (CMAKE_SIZEOF_VOID_P MATCHES 8)
	        set(EXPECT_MKL_LIBPATH "${MKLROOT_PATH}/lib/intel64")
	    else (CMAKE_SIZEOF_VOID_P MATCHES 8)
	        set(EXPECT_MKL_LIBPATH "${MKLROOT_PATH}/lib/ia32")
	    endif (CMAKE_SIZEOF_VOID_P MATCHES 8)
	endif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	
	#message(STATUS "EXPECT_MKL_LIBPATH:")
	#message(STATUS ${EXPECT_MKL_LIBPATH})
	#message(STATUS " ")

	# set include
	
	set(EXPECT_MKL_INCPATH "${MKLROOT_PATH}/include")
	
	if (IS_DIRECTORY ${EXPECT_MKL_INCPATH})
	    set(MKL_INCLUDE_DIR ${EXPECT_MKL_INCPATH})
	    set(MKL_INCLUDE_DIRS ${EXPECT_MKL_INCPATH})
	endif (IS_DIRECTORY ${EXPECT_MKL_INCPATH})
	
	if (IS_DIRECTORY ${EXPECT_MKL_LIBPATH})
		set(MKL_LIBRARY_DIR ${EXPECT_MKL_LIBPATH})
	endif (IS_DIRECTORY ${EXPECT_MKL_LIBPATH})
	
	# find specific library files
		
	find_library(LIB_BLAS NAMES mkl_blas95_ilp64 HINTS ${MKL_LIBRARY_DIR})
	find_library(LIB_CORE NAMES mkl_core HINTS ${MKL_LIBRARY_DIR})
	find_library(LIB_LAPACK NAMES mkl_lapack95_ilp64 HINTS ${MKL_LIBRARY_DIR})
	
	#find_library(LIB_MKL_RT NAMES mkl_rt HINTS ${MKL_LIBRARY_DIR})
	#find_library(LIB_PTHREAD NAMES pthread)	
	#find_library(LIB_IMF NAMES imf HINTS ${MKL_LIBRARY_DIR} ${EXPECT_ICC_LIBPATH})
	find_library(LIB_ILP64 NAMES mkl_intel_ilp64 HINTS ${MKL_LIBRARY_DIR})
	find_library(LIB_INTELTHREAD NAMES mkl_intel_thread HINTS ${MKL_LIBRARY_DIR})
	
endif (MKLROOT_PATH)

set(MKL_LIBRARIES 
	#${LIB_MKL_MC} 
	#${LIB_MKL_MC3} 
	#${LIB_MKL_P4N} 
	#${LIB_MKL_RT} 
	#${LIB_PTHREAD}
	#${LIB_IMF}
	${LIB_CORE}
	${LIB_BLAS}
	${LIB_LAPACK}
	${LIB_ILP64}
	${LIB_INTELTHREAD}
)
	
# deal with QUIET and REQUIRED argument

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(MKL DEFAULT_MSG 
    MKL_LIBRARY_DIR
    LIB_CORE
    LIB_BLAS
    LIB_LAPACK
    LIB_ILP64
    LIB_INTELTHREAD
    MKL_INCLUDE_DIR)
    
mark_as_advanced(LIB_BLAS LIB_CORE LIB_LAPACK LIB_ILP64 LIB_INTELTHREAD)
mark_as_advanced(MKL_LIBRARIES MKL_INCLUDE_DIRS MKL_INCLUDE_DIR MKL_INCLUDE_DIRS)

