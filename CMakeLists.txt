cmake_minimum_required(VERSION 3.10.2)
project(TrainTicketSystem)
set(CMAKE_CXX_STANDARD 17)
# ------------------the following is to be modified------------------
set(lib_dir_names )
set(program_names code)

# -------------------------------------------------------------------

# compile with debug info on local machine and compile with O2 on OJ
if(LOCALTEST)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/runtime)
  add_compile_options(-std=c++2a -g)
  message("!!!local!!!")
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
  add_compile_options(-std=c++2a -o2)
  add_definitions(-DONLINE_JUDGE)
endif()

# generate the dir and file lists lib_dirs : a list of the dir which the head
# files and source files are in src_files : a list of all the source file to be
# compiled
set(src_base_dir ${CMAKE_SOURCE_DIR}/src)
foreach(lib_dir_name ${lib_dir_names})
  set(now_lib_dir ${src_base_dir}/${lib_dir_name})
  list(APPEND lib_dirs ${now_lib_dir})
  file(GLOB_RECURSE tmp_files ${now_lib_dir}/*.cpp)
  list(APPEND src_files ${tmp_files})
endforeach()
# include_directories("${lib_dirs}")
include_directories(${src_base_dir})

message("${lib_dirs}")
message("${src_files}")
set(main_base_dir ${src_base_dir})
foreach(program_name ${program_names})
  set(now_main_file ${main_base_dir}/${program_name}.cpp )
  message("${program_name} ${now_main_file} ${src_files}")
  add_executable(${program_name} ${now_main_file} ${src_files})
endforeach()
