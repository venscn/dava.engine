set(CMAKE_Fortran_FLAGS_INIT "")
set(CMAKE_Fortran_FLAGS_DEBUG_INIT "-g")
set(CMAKE_Fortran_FLAGS_MINSIZEREL_INIT "-Os")
set(CMAKE_Fortran_FLAGS_RELEASE_INIT "-O3")
set(CMAKE_Fortran_FLAGS_RELWITHDEBINFO_INIT "-O2 -g")
set(CMAKE_Fortran_MODDIR_FLAG "-module ")
set(CMAKE_Fortran_VERBOSE_FLAG "-v")
set(CMAKE_Fortran_FORMAT_FIXED_FLAG "-fixed")
set(CMAKE_Fortran_FORMAT_FREE_FLAG "-free")

set(CMAKE_Fortran_CREATE_PREPROCESSED_SOURCE "<CMAKE_Fortran_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -E <SOURCE> > <PREPROCESSED_SOURCE>")
set(CMAKE_Fortran_CREATE_ASSEMBLY_SOURCE "<CMAKE_Fortran_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -S <SOURCE> -o <ASSEMBLY_SOURCE>")
