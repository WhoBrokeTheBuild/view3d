#ifndef VIEW3D_DEBUG_H
#define VIEW3D_DEBUG_H

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

// clang-format off

#if defined(DEBUG) || defined(_DEBUG)

#  define DEBUG_INFO(M, ...) \
          fprintf(stdout, "[INFO](%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__);

#  define DEBUG_WARN(M, ...) \
          fprintf(stderr, "[WARN](%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__);

#  define DEBUG_ERROR(M, ...) \
          fprintf(stderr, "[ERROR](%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__);

#else
#  define DEBUG_INFO(M, ...) do { } while(false)
#  define DEBUG_WARN(M, ...) do { } while(false)
#  define DEBUG_ERROR(M, ...) do { } while(false)
#endif

// clang-format on

#endif // VIEW3D_DEBUG_H
