ADD_LIBRARY (index SHARED
        mystorage.cpp
        index.cxx
        myimplicator.cpp
)
set_target_properties(index PROPERTIES LINKER_LANGUAGE CXX)
TARGET_LINK_LIBRARIES(index
	${ATOMSPACE_LIBRARIES}
        ${GUILE_LIBRARIES}
)

add_executable(example1
    example1.cpp)

TARGET_LINK_LIBRARIES(example1
        ${ATOMSPACE_LIBRARIES}
        ${GUILE_LIBRARIES}
        index
)

INSTALL (FILES
        index.h
	DESTINATION "include/${PROJECT_NAME}/pattern-index"
)

INSTALL (TARGETS index DESTINATION "lib${LIB_DIR_SUFFIX}/opencog")

