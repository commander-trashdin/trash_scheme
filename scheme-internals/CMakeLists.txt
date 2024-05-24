# Add library for scheme internals
add_library(scheme_internals 
builtins.cpp  cell.cpp  function.h    lisperrors.cpp  lispstring.cpp  number.cpp  objects.h  scope.h          specialform.h  symbol.cpp  util.cpp
builtins.h    cell.h    function.cpp    interfaces.h  lisperrors.h    lispstring.h    number.h    scope.cpp  specialform.cpp  storage.h      symbol.h    util.h
)

# Include directories and link dependencies
target_include_directories(scheme_internals PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/../gc)

target_link_libraries(scheme_internals PUBLIC scheme_parser scheme_tokenizer)
