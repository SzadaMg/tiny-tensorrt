#ifndef _ENTROY_CALIBRATOR_H
#define _ENTROY_CALIBRATOR_H

#include "NvInfer.h"
#include "NvInferVersion.h"
#include "utils.h"
#include <cudnn.h>
#include <string>
#include <vector>

// Version compatibility for TRT 8.0-10.x
#if NV_TENSORRT_MAJOR >= 8
#define TRT_NOEXCEPT noexcept
#else
#define TRT_NOEXCEPT
#endif

nvinfer1::IInt8Calibrator *
GetInt8Calibrator(const std::string &calibratorType, int batchSize,
                  const std::string &dataPath,
                  const std::string &calibrateCachePath);

class TrtInt8Calibrator : public nvinfer1::IInt8Calibrator {
public:
  TrtInt8Calibrator(const std::string &calibratorType, const int batchSize,
                    const std::string &dataPath,
                    const std::string &calibrateCachePath);

  virtual ~TrtInt8Calibrator();

  int getBatchSize() const TRT_NOEXCEPT override;

  bool getBatch(void *bindings[], const char *names[],
                int nbBindings) TRT_NOEXCEPT override;

  const void *readCalibrationCache(size_t &length) TRT_NOEXCEPT override;

  void writeCalibrationCache(const void *cache,
                             size_t length) TRT_NOEXCEPT override;

  nvinfer1::CalibrationAlgoType getAlgorithm() TRT_NOEXCEPT override;

#if NV_TENSORRT_MAJOR >= 10
  // Required for TensorRT 10.x (IVersionedInterface)
  nvinfer1::InterfaceInfo getInterfaceInfo() const TRT_NOEXCEPT override;
#endif

private:
  std::string mCalibratorType;
  int mBatchSize;
  std::vector<std::string> mFileList;
  std::string mCalibrateCachePath;
  int mCurBatchIdx = 0;
  int mCount;
  std::vector<void *> mDeviceBatchData;
  std::vector<char> mCalibrationCache;
};

#endif //_ENTROY_CALIBRATOR_H
