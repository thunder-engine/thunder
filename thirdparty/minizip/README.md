# Minizip. Portable zip & unzip lib 

Based on the original work of [Gilles Vollant](http://www.winimage.com/zLibDll/minizip.html)

### Usage in a CMake project

add_subdirectory (minizip)
target_link_libraries(${PROJECT_NAME} minizip)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/minizip)

In your code you can use it like

#include <minizip/zip.h>

### Requirement:

* Zlib. The Zlib library allows to deflate compressed files and to create gzip (.gz) files. Zlib is free software and small.

### Usage of library

```
#include <minizip/miniunz.h>

unzip(src, dst);
```
