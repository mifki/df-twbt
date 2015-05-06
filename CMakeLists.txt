PROJECT( twbt )

SET( PROJECT_SRCS twbt.cpp )
SET( TWBT_VER "5.xx" )

STRING( REGEX REPLACE "\\." "" DV_VS "${DF_VERSION}" )

SET( CMAKE_CXX_FLAGS "-m32 -O3 -DDFHACK_VERSION=\"${DFHACK_VERSION}\" -DDF_${DV_VS} -DTWBT_VER=\"\\\"${TWBT_VER}\\\"\"" )

IF( APPLE )
  SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x -stdlib=libstdc++ -Wno-tautological-compare -framework OpenGL -mmacosx-version-min=10.6 -undefined dynamic_lookup" )
ELSE( APPLE )
  SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x" )
ENDIF( APPLE )

DFHACK_PLUGIN( twbt ${PROJECT_SRCS} )

# add_subdirectory( plugins ) Already included in dfhack?
