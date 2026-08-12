cmake_minimum_required(VERSION 3.15)

project(Dandelifeon VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_subdirectory(lua)

add_executable(Dandelifeon src/main.cpp)

target_include_directories(Dandelifeon PRIVATE src)

target_link_libraries(Dandelifeon PRIVATE lua_static)


if(MSVC)
    target_compile_options(Dandelifeon PRIVATE /arch:AVX2)
    
    target_compile_options(Dandelifeon PRIVATE 
        $<$<NOT:$<CONFIG:Debug>>:/O2>
        $<$<NOT:$<CONFIG:Debug>>:/Oi>
        $<$<NOT:$<CONFIG:Debug>>:/Ot>
        $<$<NOT:$<CONFIG:Debug>>:/GL>
    )
    
    target_link_options(Dandelifeon PRIVATE 
        $<$<NOT:$<CONFIG:Debug>>:/LTCG>
    )
else()
    target_compile_options(Dandelifeon PRIVATE -mavx2)
    
    target_compile_options(Dandelifeon PRIVATE 
        $<$<NOT:$<CONFIG:Debug>>:-O3>
        $<$<NOT:$<CONFIG:Debug>>:-march=native>
        $<$<NOT:$<CONFIG:Debug>>:-flto>
    )
    
    target_link_options(Dandelifeon PRIVATE 
        $<$<NOT:$<CONFIG:Debug>>:-flto>
    )
endif()

add_custom_command(TARGET Dandelifeon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/scripts"
        "$<TARGET_FILE_DIR:Dandelifeon>/scripts"
)