/*
 * Copyright 2024 Vision Labs LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *    http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cuda.h>

namespace VPF {
/* Check CUDA driver API return code, throw std::runtime_error with description
 * in case of failure;
 */
void ThrowOnCudaError(CUresult res, int lineNum);

/* Get CUDA context associated with given device pointer.
 * May throw exception with reason in message;
 */
CUcontext GetContextByDptr(CUdeviceptr dptr);

/* Get CUDA device id for given CUdeviceptr;
 * May throw exception with reason in message;
 */
int GetDeviceIdByDptr(CUdeviceptr dptr);

/* RAII-style CUDA Context sync;
 */
class TC_EXPORT CudaStrSync final {
  CUstream str;

public:
  explicit CudaStrSync(CUstream stream);
  ~CudaStrSync();
};

/* RAII-style CUDA Context (un)lock;
 */
class TC_EXPORT CudaCtxPush final {
public:
  explicit CudaCtxPush(CUcontext ctx);
  ~CudaCtxPush();
};

/* RAII-style CUDA event wrapper;
 */
class TC_EXPORT CudaStreamEvent final {
  CUevent m_event;

public:
  CudaStreamEvent() = delete;
  CudaStreamEvent& operator=(const CudaStreamEvent&) = delete;
  CudaStreamEvent(CUstream stream);
  ~CudaStreamEvent();

  void Wait();
  inline CUevent Get() noexcept { return m_event; }
};
} // namespace VPF