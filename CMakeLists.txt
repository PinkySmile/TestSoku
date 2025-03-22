cmake_minimum_required(VERSION 3.15)
project(SokuTest)

set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(_DEBUG)
add_compile_options(-g -O0 -W -Wall -Wextra -Wshadow)
add_executable(
	SokuTest
	src/main.cpp
	src/Packet.hpp
	src/Client.cpp
	src/Client.hpp
	src/RemoteClient.cpp
	src/RemoteClient.hpp
	src/Vector2.hpp
	src/Packet.cpp
	src/Exceptions.hpp
)

if (WIN32)
	target_link_libraries(
		SokuTest
		sfml-network
		ws2_32
	)
else ()
	target_link_libraries(
		SokuTest
		sfml-network
	)
endif()

add_executable(
	sniff
	src/sniff.cpp
	src/Packet.hpp
	src/Client.cpp
	src/Client.hpp
	src/Vector2.hpp
	src/Packet.cpp
	src/Exceptions.hpp
	src/RemoteClient.cpp
	src/RemoteClient.hpp
)

if (WIN32)
	target_link_libraries(
		sniff
		sfml-network
		ws2_32
	)
else ()
	target_link_libraries(
		sniff
		sfml-network
	)
endif()

add_executable(
	forward
	src/forward.cpp
	src/Packet.hpp
	src/Client.cpp
	src/Client.hpp
	src/Vector2.hpp
	src/Packet.cpp
	src/Exceptions.hpp
	src/RemoteClient.cpp
	src/RemoteClient.hpp
)

if (WIN32)
	target_link_libraries(
		forward
		sfml-network
		ws2_32
	)
else ()
	target_link_libraries(
		forward
		sfml-network
	)
endif()