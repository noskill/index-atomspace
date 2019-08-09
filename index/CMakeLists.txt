ADD_LIBRARY (index SHARED
        mystorage.cpp
        index.cxx
)
set_target_properties(index PROPERTIES LINKER_LANGUAGE CXX)
TARGET_LINK_LIBRARIES(index
	${ATOMSPACE_LIBRARIES}
)


INSTALL (FILES
        index.h
	DESTINATION "include/${PROJECT_NAME}/pattern-index"
)

INSTALL (TARGETS index DESTINATION "lib${LIB_DIR_SUFFIX}/opencog")

