############################################################################
# CMakeLists.txt file for building ROOT rootx package
# @author Pere Mato, CERN
############################################################################

if(x11)
  ROOT_EXECUTABLE(root
    src/rootx.cxx
    src/rootxx.cxx
    ../core/clib/src/strlcpy.c
  )
  target_include_directories(root
    PRIVATE
      ${X11_INCLUDE_DIR}
  )
  target_link_libraries(root
    PRIVATE
      ${X11_Xft_LIB}
      ${X11_Xpm_LIB}
      ${X11_LIBRARIES}
  )
elseif(cocoa)
  if (cxxmodules)
    # FIXME: Disable modules for ObjC/ObjC++. It has problems when compiling
    # rootxx-cocoa.mm with a lots of warnings and errors such as:
    # rootxx-cocoa.mm:884:48: error: property 'length' not found on object of type 'NSMutableAttributedString *'
    #
    # FIXME: We should disable building with modules on only the TU of rootxx-cocoa.mm
    # Unfortunately, cmake cannot reliably remove flags from a single TU.
    # as COMPILE_OPTIONS and COMPILE_FLAGS could be overriden causing tricky to
    # debug problems.
    string(REPLACE "${ROOT_CXXMODULES_CXXFLAGS}" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REPLACE "${ROOT_CXXMODULES_CFLAGS}" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
  endif(cxxmodules)

  # FIXME: rootxx-cocoa.mm should be compiled with -ObjC++ flag. Here we rely
  # that the compiler will recognise the extension mm and switch to the correct
  # language mode.
  ROOT_EXECUTABLE(root
    src/rootx.cxx
    src/rootxx-cocoa.mm
    LIBRARIES
      "-framework Cocoa"
  )
endif()

if(x11 OR cocoa)
  generateHeaders(${CMAKE_SOURCE_DIR}/core/base/src/root-argparse.py
                  ${CMAKE_BINARY_DIR}/include/rootCommandLineOptionsHelp.h root)
endif()
