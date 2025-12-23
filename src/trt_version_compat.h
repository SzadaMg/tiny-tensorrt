/*
 * TensorRT Version Compatibility Header
 * This header centralizes version-specific macros and compatibility handling
 * for TensorRT versions 8.0 through 10.x
 */

#ifndef TRT_VERSION_COMPAT_H
#define TRT_VERSION_COMPAT_H

#include "NvInferVersion.h"

// Version helper macros
#define TRT_VERSION_GE(major, minor, patch)                                    \
  ((NV_TENSORRT_MAJOR > (major)) ||                                            \
   (NV_TENSORRT_MAJOR == (major) && NV_TENSORRT_MINOR > (minor)) ||            \
   (NV_TENSORRT_MAJOR == (major) && NV_TENSORRT_MINOR == (minor) &&            \
    NV_TENSORRT_PATCH >= (patch)))

#define TRT_VERSION_LT(major, minor, patch)                                    \
  (!TRT_VERSION_GE(major, minor, patch))

// noexcept specifier for methods (added in TRT 8.0)
#if NV_TENSORRT_MAJOR >= 8
#define TRT_NOEXCEPT noexcept
#else
#define TRT_NOEXCEPT
#endif

// Backward compatibility alias
#ifndef IS_NOEXCEPT
#define IS_NOEXCEPT TRT_NOEXCEPT
#endif

// destroy() vs delete - TRT 8.0+ uses delete instead of destroy()
#define TRT_LEGACY_DESTROY (NV_TENSORRT_MAJOR < 8)

// Workspace size methods were deprecated in TRT 8.4
#define TRT_HAS_WORKSPACE_SIZE_API                                             \
  (NV_TENSORRT_MAJOR < 8 || (NV_TENSORRT_MAJOR == 8 && NV_TENSORRT_MINOR < 4))

// DLA support variations
#define TRT_HAS_DLA_SUPPORT (NV_TENSORRT_MAJOR >= 8)

// Dynamic shape support (available in TRT 6.0+, but we focus on 8.0+)
#define TRT_HAS_DYNAMIC_SHAPES (NV_TENSORRT_MAJOR >= 6)

// Enqueue API signature changed in TRT 8.0
#define TRT_LEGACY_ENQUEUE (NV_TENSORRT_MAJOR < 8)

#endif // TRT_VERSION_COMPAT_H
