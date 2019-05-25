# TIRC++

TIRC++ is a C++17 library for IRC communication in Twitch chat. It aims to be a reliable, efficient and portable library to simplify the creation of Twitch chat bots. Additionally, APIs in several languages such as C, Java and Python, should eventually be supported.

## Getting Started

To build the TIRC++ library, create a `build` directory inside the project folder, navigate to it and run

```
cmake ..
```

To build a release version (which prints no logs of any kind) add the `-DCMAKE_BUILD_TYPE=Release` option to the command above.

### Prerequisites

To Build TIRC++ you need CMake 3.10+. To install it use your system's package manager or [download it here](https://cmake.org/download/) and follow the [installation instructions here](https://cmake.org/install).

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.
