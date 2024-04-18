/*
 * Copyright 2019 NVIDIA Corporation
 * Copyright 2021 Videonetics Technology Private Limited
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

#include <stdexcept>

#include "MemoryInterfaces.hpp"
#include "Tasks.hpp"

extern "C" {
#include <libavutil/pixdesc.h>
}

using namespace VPF;

auto const cuda_stream_sync = [](void* stream) {
  cuStreamSynchronize((CUstream)stream);
};

namespace VPF {
struct RemapSurface_Impl {
  std::shared_ptr<Surface> pSurface = nullptr;
  std::shared_ptr<CudaBuffer> xMapPlane = nullptr;
  std::shared_ptr<CudaBuffer> yMapPlane = nullptr;
  CUcontext cu_ctx;
  CUstream cu_str;
  NppStreamContext nppCtx;

  uint32_t map_w;
  uint32_t map_h;

  RemapSurface_Impl(const float* x_map, const float* y_map, uint32_t remap_w,
                    uint32_t remap_h, Pixel_Format format, CUcontext ctx,
                    CUstream str)
      : cu_ctx(ctx), cu_str(str), map_w(remap_w), map_h(remap_h) {
    xMapPlane.reset(CudaBuffer::Make((const void*)x_map, sizeof(float),
                                     map_w * map_h, ctx, str));
    yMapPlane.reset(CudaBuffer::Make((const void*)y_map, sizeof(float),
                                     map_w * map_h, ctx, str));

    SetupNppContext(cu_ctx, cu_str, nppCtx);
  }

  virtual ~RemapSurface_Impl() = default;
  virtual TaskExecStatus Run(Surface& source) = 0;
};

struct NppRemapSurfacePacked3C_Impl final : RemapSurface_Impl {
  NppRemapSurfacePacked3C_Impl(const float* x_map, const float* y_map,
                               uint32_t remap_w, uint32_t remap_h,
                               CUcontext ctx, CUstream str, Pixel_Format format)
      : RemapSurface_Impl(x_map, y_map, remap_w, remap_h, format, ctx, str) {
    pSurface = Surface::Make(format, map_w, map_h, ctx);
  }

  ~NppRemapSurfacePacked3C_Impl() = default;

  TaskExecStatus Run(Surface& source) {
    NvtxMark tick("NppRemapSurfacePacked3C");

    if (pSurface->PixelFormat() != source.PixelFormat()) {
      return TaskExecStatus::TASK_EXEC_FAIL;
    }

    try {
      auto src_ctx = source.GetNppContext();
      auto dst_ctx = pSurface->GetNppContext();

      CudaCtxPush ctxPush(cu_ctx);
      auto ret = nppiRemap_8u_C3R_Ctx(
          src_ctx.GetDataAs<const Npp8u>()[0], src_ctx.GetSize(),
          src_ctx.GetPitch()[0], src_ctx.GetRect(),
          (const Npp32f*)xMapPlane->GpuMem(), map_w * xMapPlane->GetElemSize(),
          (const Npp32f*)yMapPlane->GpuMem(), map_w * yMapPlane->GetElemSize(),
          dst_ctx.GetDataAs<Npp8u>()[0], dst_ctx.GetPitch()[0],
          dst_ctx.GetSize(), (int)NPPI_INTER_LINEAR, nppCtx);

      if (NPP_NO_ERROR != ret) {
        return TaskExecStatus::TASK_EXEC_FAIL;
      }
    } catch (...) {
      return TaskExecStatus::TASK_EXEC_FAIL;
    }

    return TaskExecStatus::TASK_EXEC_SUCCESS;
  }
};
} // namespace VPF

RemapSurface::RemapSurface(const float* x_map, const float* y_map,
                           uint32_t remap_w, uint32_t remap_h,
                           Pixel_Format format, CUcontext ctx, CUstream str)
    : Task("NppRemapSurface", RemapSurface::numInputs, RemapSurface::numOutputs,
           cuda_stream_sync, (void*)str) {
  if (RGB == format || BGR == format) {
    pImpl = new NppRemapSurfacePacked3C_Impl(x_map, y_map, remap_w, remap_h,
                                             ctx, str, format);
  } else {
    throw std::runtime_error("pixel format not supported");
  }
}

RemapSurface::~RemapSurface() { delete pImpl; }

TaskExecStatus RemapSurface::Run() {
  NvtxMark tick(GetName());
  ClearOutputs();

  auto pInputSurface = (Surface*)GetInput();
  if (!pInputSurface) {
    return TaskExecStatus::TASK_EXEC_FAIL;
  }

  if (TaskExecStatus::TASK_EXEC_SUCCESS != pImpl->Run(*pInputSurface)) {
    return TaskExecStatus::TASK_EXEC_FAIL;
  }

  SetOutput(pImpl->pSurface.get(), 0U);
  return TaskExecStatus::TASK_EXEC_SUCCESS;
}

RemapSurface* RemapSurface::Make(const float* x_map, const float* y_map,
                                 uint32_t remap_w, uint32_t remap_h,
                                 Pixel_Format format, CUcontext ctx,
                                 CUstream str) {
  return new RemapSurface(x_map, y_map, remap_w, remap_h, format, ctx, str);
}
