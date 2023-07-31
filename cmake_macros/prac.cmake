macro(SETUP_GROUPS src_files)
foreach(FILE ${src_files})
	get_filename_component(PARENT_DIR "${FILE}" PATH)

	# skip src or include and changes /'s to \\'s
	set(GROUP "${PARENT_DIR}")
	string(REPLACE "/" "\\" GROUP "${GROUP}")


	source_group("${GROUP}" FILES "${FILE}")
endforeach()
endmacro()

macro(SET_OUTPUT_NAMES projname)
set_target_properties(${projname} PROPERTIES OUTPUT_NAME_DEBUG ${projname}_Debug)
set_target_properties(${projname} PROPERTIES OUTPUT_NAME_RELEASE ${projname}_Release)
set_target_properties(${projname} PROPERTIES OUTPUT_NAME_RELWITHDEBINFO ${projname}_ReleaseDebInfo)

# On Linux/macOS the binaries go to <root>/bin folder
if (UNIX)
	set_target_properties(${projname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
endif()
endmacro()

macro(SETUP_APP projname chapter)
	set(FOLDER_NAME ${chapter})
	set(PROJECT_NAME ${projname})
	# project(${PROJECT_NAME} CXX)

	file(GLOB_RECURSE SRC_FILES 	LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.cpp)
	file(GLOB_RECURSE HEADER_FILES 	LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h *.hpp)

	add_executable(${PROJECT_NAME} ${SRC_FILES} ${HEADER_FILES})
	if(MSVC)
		add_definitions(-D_CONSOLE)
	endif()
	SETUP_GROUPS("${SRC_FILES}")
	SETUP_GROUPS("${HEADER_FILES}")

	SET_OUTPUT_NAMES(${PROJECT_NAME})

	# if(NOT MSVC)
	# 	set_property(TARGET ${PROJECT_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
	# endif()

	set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER ${FOLDER_NAME})

	set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
	set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

	# if(MSVC)
	# 	set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
	# endif()

endmacro()

macro(subdirlist result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

macro(GET_SRC_FILES SRC_FILES HEADER_FILES)
file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c??)
file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h??)
endmacro()

macro(LINK_SIMPLE_LIB)
	set(ALL_FILES "")
	GET_SRC_FILES(SRC_FILES HEADER_FILES)

	foreach(CXX_FILE ${SRC_FILES})
		list(APPEND ALL_FILES ${CXX_FILE})    
	endforeach()

	foreach(CXX_FILE ${HEADER_FILES})
		list(APPEND ALL_FILES ${CXX_FILE})    
	endforeach()

	get_filename_component(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR} NAME)
	string(TOLOWER ${LIB_DIR} LIB_DIR)

	add_library(${LIB_DIR} OBJECT ${ALL_FILES})
	target_link_directories(${LIB_DIR} PUBLIC .)
endmacro()
