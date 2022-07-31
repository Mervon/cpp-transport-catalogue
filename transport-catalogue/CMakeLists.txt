cmake_minimum_required(VERSION 3.11)

project(14th_sprint CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto)

add_executable(transport_catalogue main.cpp ${PROTO_SRCS} ${PROTO_HDRS} domain.cpp domain.h geo.cpp geo.h graph.h json.h json.cpp
    json_builder.h json_builder.cpp map_renderer.h map_renderer.cpp ranges.h router.h 
    request_handler.cpp request_handler.h svg.h svg.cpp transport_catalogue.cpp transport_catalogue.h
    transport_router.h transport_router.cpp json_reader.h json_reader.cpp transport_catalogue.proto
    serialization.h serialization.cpp)

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)