# TensorRT Version Compatibility Guide

This document describes the TensorRT version compatibility features in tiny-tensorrt, covering TensorRT 8.0 through TensorRT 10.x.

## Overview

The codebase has been refactored to support both TensorRT 8.0 and TensorRT 10.x using preprocessor macros and conditional compilation. This ensures backward compatibility while taking advantage of newer TensorRT features when available.

## Supported TensorRT Versions

- **TensorRT 8.0** - Fully supported
- **TensorRT 8.x** (8.1, 8.2, 8.3) - Fully supported
- **TensorRT 8.4+** - Fully supported (with workspace API deprecation handling)
- **TensorRT 9.x** - Supported (same API as TensorRT 8.x)
- **TensorRT 10.x** - Fully supported

## Key API Differences Handled

### 1. Object Destruction (`destroy()` vs `delete`)

**TensorRT 7.x and earlier:**
```cpp
builder->destroy();
```

**TensorRT 8.0+ (including TRT 10):**
```cpp
delete builder;
```

**Implementation:** The `TrtDestroyer` template in `Trt.h` automatically handles this:
```cpp
template <typename T>
struct TrtDestroyer {
    void operator()(T* t) {
#if NV_TENSORRT_MAJOR < 8
        t->destroy();
#else
        delete t;
#endif
    }
};
```

### 2. `noexcept` Specifier

**TensorRT 8.0+** requires `noexcept` on certain virtual method overrides.

**Implementation:** The `TRT_NOEXCEPT` macro is defined throughout the codebase:
```cpp
#if NV_TENSORRT_MAJOR >= 8
    #define TRT_NOEXCEPT noexcept
#else
    #define TRT_NOEXCEPT
#endif
```

This is used in:
- `Trt.h` - TrtLogger::log()
- `plugin_utils.h` - Plugin utility functions
- `Int8Calibrator.h/.cpp` - Calibrator methods
- All plugin implementations

### 3. Workspace Size API (Deprecated in TRT 8.4, Removed in TRT 10)

**TensorRT 8.0-8.3:**
```cpp
config->setMaxWorkspaceSize(1 << 30);
```

**TensorRT 8.4+ and TRT 10:**
```cpp
// Use setMemoryPoolLimit() instead
config->setMemoryPoolLimit(MemoryPoolType::kWORKSPACE, 1 << 30);
```

**Implementation:** Conditional compilation is used:
```cpp
#if !(NV_TENSORRT_MAJOR >= 8 && NV_TENSORRT_MINOR >= 4)
void Trt::SetWorkpaceSize(size_t workspaceSize) {
    mConfig->setMaxWorkspaceSize(workspaceSize);
}
#endif
```

### 4. Engine Building API

**TensorRT 7.x:**
```cpp
engine = builder->buildEngineWithConfig(*network, *config);
plan = engine->serialize();
```

**TensorRT 8.0+ (including TRT 10):**
```cpp
plan = builder->buildSerializedNetwork(*network, *config);
engine = runtime->deserializeCudaEngine(plan->data(), plan->size());
```

**Implementation:** Version checks in `Trt.cpp`:
```cpp
#if NV_TENSORRT_MAJOR < 8
    mEngine.reset(mBuilder->buildEngineWithConfig(*network, *mConfig));
    TrtUniquePtr<nvinfer1::IHostMemory> plan{mEngine->serialize()};
#else
    TrtUniquePtr<nvinfer1::IHostMemory> plan{mBuilder->buildSerializedNetwork(*network, *mConfig)};
    mEngine.reset(runtime->deserializeCudaEngine(plan->data(), plan->size()));
#endif
```

### 5. Plugin Enqueue Method Signature

**TensorRT 7.x:**
```cpp
int enqueue(int batchSize, const void *const *inputs, void **outputs,
            void *workspace, cudaStream_t stream);
```

**TensorRT 8.0+ (including TRT 10):**
```cpp
int enqueue(int batchSize, void const * const * inputs, void * const * outputs,
            void* workspace, cudaStream_t stream) noexcept;
```

**Implementation:** Conditional method declarations in plugin headers:
```cpp
#if NV_TENSORRT_MAJOR < 8
    int enqueue(int batchSize, const void *const *inputs, void **outputs,
                void *workspace, cudaStream_t stream) override;
#else
    int enqueue(int batchSize, void const * const * inputs, void * const * outputs,
                void* workspace, cudaStream_t stream) noexcept override;
#endif
```

## Version Compatibility Headers

### Core Compatibility Header: `src/trt_version_compat.h`

This header provides centralized version checking utilities:

```cpp
// Version helper macros
#define TRT_VERSION_GE(major, minor, patch) \
    ((NV_TENSORRT_MAJOR > (major)) || ...)

#define TRT_VERSION_LT(major, minor, patch) (...)

// Feature availability macros
#define TRT_LEGACY_DESTROY (NV_TENSORRT_MAJOR < 8)
#define TRT_HAS_WORKSPACE_SIZE_API (NV_TENSORRT_MAJOR < 8 || ...)
#define TRT_LEGACY_ENQUEUE (NV_TENSORRT_MAJOR < 8)
```

### Files Updated for Compatibility

1. **Trt.h** - Core TensorRT wrapper header
   - Added `TRT_NOEXCEPT` macro
   - Updated `TrtDestroyer` for TRT 8.0+
   - Improved workspace API documentation

2. **Trt.cpp** - Core TensorRT wrapper implementation
   - Added version compatibility includes
   - Updated engine building logic
   - Improved version-specific comments

3. **plugin_utils.h** - Plugin utilities
   - Added `TRT_NOEXCEPT` and `IS_NOEXCEPT` macros
   - Added `NvInferVersion.h` include

4. **Int8Calibrator.h/.cpp** - INT8 calibration
   - Updated all method signatures with `TRT_NOEXCEPT`

5. **CuteSamplePlugin.h/.cpp** - Sample plugin
   - Updated for TRT 8.0-10.x compatibility
   - Added version-specific enqueue signatures

6. **CuteSampleDynamicExtPlugin.h/.cpp** - Dynamic shape plugin
   - Updated compatibility macros
   - Improved version comments

## Building for Different TensorRT Versions

### CMake Configuration

The build system automatically detects the TensorRT version via `NvInferVersion.h`:

```bash
cmake -DGPU_ARCH=75 \
      -DTENSORRT_INCLUDE_PATH=/usr/local/tensorrt/include \
      -DTENSORRT_LIB_PATH=/usr/local/tensorrt/lib \
      ..
```

### Checking Version at Runtime

You can check the TensorRT version at compile time:
```cpp
#if NV_TENSORRT_MAJOR >= 10
    // TRT 10.x specific code
#elif NV_TENSORRT_MAJOR >= 8
    // TRT 8.x-9.x code
#else
    // TRT 7.x code
#endif
```

## Migration Notes

### Migrating from TRT 8.0-8.3 to TRT 8.4+/10.x

1. **Workspace Size:**
   - Replace `setMaxWorkspaceSize()` with `setMemoryPoolLimit()`
   - The old API is conditionally compiled out in TRT 8.4+

2. **Build Configuration:**
   - No code changes needed; version checks handle this automatically

3. **Plugins:**
   - All plugin implementations use version-aware macros
   - No manual changes needed

### Upgrading from TRT 7.x to TRT 8.0+

If you're upgrading from TRT 7.x:
1. The codebase requires TRT 7.0+ minimum
2. All TRT 8.0+ changes are handled automatically
3. Ensure your CUDA and cuDNN versions are compatible with your TensorRT version

## Testing Compatibility

To ensure your build works with your TensorRT version:

```bash
# Build the project
mkdir build && cd build
cmake -DGPU_ARCH=<your_arch> \
      -DTENSORRT_INCLUDE_PATH=<path> \
      -DTENSORRT_LIB_PATH=<path> \
      -DBUILD_SAMPLE=ON ..
make

# Run samples to verify
./samples/tinyexec/tinyexec --help
```

## Known Issues and Limitations

1. **TensorRT 7.x Support:** While basic structures exist, TRT 7.x is not actively tested. TRT 8.0+ is recommended.

2. **Workspace API:** The deprecated `SetWorkpaceSize()` method is only available when building with TRT 8.0-8.3. For TRT 8.4+, users must implement memory pool limits directly through the config object.

3. **DLA Support:** DLA-specific features are only available in TRT 8.0+.

## Best Practices

1. **Use Version Macros:** Always use the provided macros (`TRT_NOEXCEPT`, `IS_NOEXCEPT`) rather than hardcoding `noexcept`.

2. **Check Feature Availability:** Use version checks to ensure features are available:
   ```cpp
   #if !(NV_TENSORRT_MAJOR >= 8 && NV_TENSORRT_MINOR >= 4)
       // Old API code
   #else
       // New API code
   #endif
   ```

3. **Stay Updated:** Monitor TensorRT release notes for deprecated features and plan migrations accordingly.

4. **Test Across Versions:** If possible, test your builds with multiple TensorRT versions to ensure compatibility.

## Additional Resources

- [TensorRT Developer Guide](https://docs.nvidia.com/deeplearning/tensorrt/developer-guide/)
- [TensorRT API Documentation](https://docs.nvidia.com/deeplearning/tensorrt/api/)
- [TensorRT Release Notes](https://docs.nvidia.com/deeplearning/tensorrt/release-notes/)

## Contributing

When adding new features, ensure compatibility with TensorRT 8.0-10.x:
1. Use the existing version macros
2. Document version-specific behavior
3. Test with multiple TensorRT versions when possible
4. Update this document with any new compatibility considerations
