cmake_minimum_required(VERSION 3.16)
project(ChatApplication)

set(CMAKE_CXX_STANDARD 17)
include(FetchContent)

# Optionally include only the needed Boost libs
set(BOOST_ENABLE_CMAKE ON)
set(BOOST_INCLUDE_LIBRARIES asio system align assert)

FetchContent_Declare(
  Boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.88.0/boost-1.88.0-cmake.zip
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)

FetchContent_MakeAvailable(Boost)

# ---------- Server ----------
add_executable(chat_server
    Src/main_server.cpp
    Src/Server.cpp
    Src/Session.cpp
    Src/ConsoleUtils.cpp
)
target_link_libraries(chat_server PRIVATE Boost::asio Boost::system)

# ---------- Client ----------
add_executable(chat_client
    Src/main_client.cpp
    Src/Client.cpp
    Src/ConsoleUtils.cpp
)
target_link_libraries(chat_client PRIVATE Boost::asio Boost::system)


