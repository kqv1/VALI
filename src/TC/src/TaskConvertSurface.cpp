/*
 * Copyright 2019 NVIDIA Corporation
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

#include "CodecsSupport.hpp"
#include "MemoryInterfaces.hpp"
#include "NppCommon.hpp"
#include "NvCodecUtils.h"
#include "Tasks.hpp"

#include <memory>
#include <sstream>
#include <stdexcept>
#include <tuple>

using namespace VPF;

constexpr auto TASK_EXEC_SUCCESS = TaskExecStatus::TASK_EXEC_SUCCESS;
constexpr auto TASK_EXEC_FAIL = TaskExecStatus::TASK_EXEC_FAIL;

namespace VPF {

struct NppConvertSurface_Impl {
  NppConvertSurface_Impl() = delete;
  NppConvertSurface_Impl(const NppConvertSurface_Impl& other) = delete;
  NppConvertSurface_Impl(const NppConvertSurface_Impl&& other) = delete;
  NppConvertSurface_Impl&
  operator=(const NppConvertSurface_Impl& other) = delete;
  NppConvertSurface_Impl&
  operator=(const NppConvertSurface_Impl&& other) = delete;

  NppConvertSurface_Impl(CUcontext ctx, CUstream str, Pixel_Format SRC_FMT,
                         Pixel_Format DST_FMT)
      : cu_ctx(ctx), cu_str(str), srcFmt(SRC_FMT), dstFmt(DST_FMT) {
    SetupNppContext(cu_ctx, cu_str, nppCtx);
    pDetails.reset(Buffer::Make(sizeof(details), (void*)&details));
  }

  virtual ~NppConvertSurface_Impl() = default;

  virtual Token* Execute(Token* pSrcToken, Token* pDstToken,
                         ColorspaceConversionContext* pCtx) = 0;

  bool Validate(Surface* pSrc, Surface* pDst) {
    if (!pSrc || !pDst) {
      return false;
    }

    if ((pSrc->PixelFormat() != srcFmt) || (pDst->PixelFormat() != dstFmt)) {
      return false;
    }

    if ((pSrc->Width() != pDst->Width()) ||
        (pSrc->Height() != pDst->Height())) {
      return false;
    }

    return true;
  }

  Surface* GetOutput(Surface* pSrc, Surface* pDst) {
    if (!pSrc) {
      return nullptr;
    }

    if (pDst) {
      return pDst;
    }

    if (!pSurface) {
      pSurface.reset(
          Surface::Make(dstFmt, pSrc->Width(), pSrc->Height(), cu_ctx));
    }

    return pSurface.get();
  }

  std::string GetNvtxTickName() const {
    std::stringstream ss("GPU_");
    ss << GetFormatName(srcFmt);
    ss << "_2_";
    ss << GetFormatName(dstFmt);

    return ss.str();
  }

  static std::tuple<ColorSpace, ColorRange>
  GetParams(ColorspaceConversionContext* pCtx) {
    auto ret = std::make_tuple(BT_601, MPEG);

    if (pCtx) {
      std::get<0>(ret) = pCtx->color_space;
      std::get<1>(ret) = pCtx->color_range;
    }

    return ret;
  }

  CUcontext cu_ctx;
  CUstream cu_str;
  NppStreamContext nppCtx;
  const Pixel_Format srcFmt, dstFmt;
  std::shared_ptr<Surface> pSurface = nullptr;
  std::shared_ptr<Buffer> pDetails = nullptr;
  TaskExecDetails details;
};

struct nv12_bgr final : public NppConvertSurface_Impl {
  nv12_bgr(uint32_t width, uint32_t height, CUcontext context, CUstream stream)
      : NppConvertSurface_Impl(context, stream, NV12, BGR) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (Surface*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    auto const params = GetParams(pCtx);
    auto const color_space = std::get<0>(params);
    auto const color_range = std::get<1>(params);

    const Npp8u* const pSrc[] = {(const Npp8u* const)pInput->PlanePtr(0U),
                                 (const Npp8u* const)pInput->PlanePtr(1U)};

    auto pDst = (Npp8u*)pOutput->PlanePtr();
    NppiSize oSizeRoi = {(int)pInput->Width(), (int)pInput->Height()};

    CudaCtxPush ctxPush(cu_ctx);
    auto err = NPP_NO_ERROR;

    switch (color_space) {
    case BT_709:
      if (JPEG == color_range) {
        err = nppiNV12ToBGR_709HDTV_8u_P2C3R_Ctx(
            pSrc, pInput->Pitch(), pDst, pOutput->Pitch(), oSizeRoi, nppCtx);
      } else {
        err = nppiNV12ToBGR_709CSC_8u_P2C3R_Ctx(
            pSrc, pInput->Pitch(), pDst, pOutput->Pitch(), oSizeRoi, nppCtx);
      }
      break;
    case BT_601:
      if (JPEG == color_range) {
        err = nppiNV12ToBGR_8u_P2C3R_Ctx(pSrc, pInput->Pitch(), pDst,
                                         pOutput->Pitch(), oSizeRoi, nppCtx);
      } else {
        details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
        return nullptr;
      }
      break;
    default:
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    }

    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct nv12_rgb final : public NppConvertSurface_Impl {
  nv12_rgb(uint32_t width, uint32_t height, CUcontext context, CUstream stream)
      : NppConvertSurface_Impl(context, stream, NV12, RGB) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (Surface*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    auto const params = GetParams(pCtx);
    auto const color_space = std::get<0>(params);
    auto const color_range = std::get<1>(params);

    const Npp8u* const pSrc[] = {(const Npp8u* const)pInput->PlanePtr(0U),
                                 (const Npp8u* const)pInput->PlanePtr(1U)};

    auto pDst = (Npp8u*)pOutput->PlanePtr();
    NppiSize oSizeRoi = {(int)pInput->Width(), (int)pInput->Height()};

    CudaCtxPush ctxPush(cu_ctx);
    auto err = NPP_NO_ERROR;

    switch (color_space) {
    case BT_709:
      if (JPEG == color_range) {
        err = nppiNV12ToRGB_709HDTV_8u_P2C3R_Ctx(
            pSrc, pInput->Pitch(), pDst, pOutput->Pitch(), oSizeRoi, nppCtx);
      } else {
        err = nppiNV12ToRGB_709CSC_8u_P2C3R_Ctx(
            pSrc, pInput->Pitch(), pDst, pOutput->Pitch(), oSizeRoi, nppCtx);
      }
      break;
    case BT_601:
      if (JPEG == color_range) {
        err = nppiNV12ToRGB_8u_P2C3R_Ctx(pSrc, pInput->Pitch(), pDst,
                                         pOutput->Pitch(), oSizeRoi, nppCtx);
      } else {
        details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
        return nullptr;
      }
      break;
    default:
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    }

    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct nv12_yuv420 final : public NppConvertSurface_Impl {
  nv12_yuv420(uint32_t width, uint32_t height, CUcontext context,
              CUstream stream)
      : NppConvertSurface_Impl(context, stream, NV12, YUV420) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (Surface*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    }

    const Npp8u* const pSrc[] = {(const Npp8u*)pInput->PlanePtr(0U),
                                 (const Npp8u*)pInput->PlanePtr(1U)};

    Npp8u* pDst[] = {(Npp8u*)pOutput->PlanePtr(0U),
                     (Npp8u*)pOutput->PlanePtr(1U),
                     (Npp8u*)pOutput->PlanePtr(2U)};

    int dstStep[] = {(int)pOutput->Pitch(0U), (int)pOutput->Pitch(1U),
                     (int)pOutput->Pitch(2U)};
    NppiSize roi = {(int)pInput->Width(), (int)pInput->Height()};

    CudaCtxPush ctxPush(cu_ctx);
    auto err = NPP_NO_ERROR;

    auto const color_range = pCtx ? pCtx->color_range : MPEG;
    switch (color_range) {
    case JPEG:
      err = nppiNV12ToYUV420_8u_P2P3R_Ctx(pSrc, pInput->Pitch(0U), pDst,
                                          dstStep, roi, nppCtx);
      break;
    case MPEG:
      err = nppiYCbCr420_8u_P2P3R_Ctx(pSrc[0], pInput->Pitch(0U), pSrc[1],
                                      pInput->Pitch(1U), pDst, dstStep, roi,
                                      nppCtx);
      break;
    default:
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    }

    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct nv12_y final : public NppConvertSurface_Impl {
  nv12_y(uint32_t width, uint32_t height, CUcontext context, CUstream stream)
      : NppConvertSurface_Impl(context, stream, NV12, Y) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (Surface*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    CUDA_MEMCPY2D m = {0};
    m.srcMemoryType = CU_MEMORYTYPE_DEVICE;
    m.dstMemoryType = CU_MEMORYTYPE_DEVICE;
    m.srcDevice = pInput->PlanePtr();
    m.dstDevice = pOutput->PlanePtr();
    m.srcPitch = pInput->Pitch();
    m.dstPitch = pOutput->Pitch();
    m.Height = pInput->Height();
    m.WidthInBytes = pInput->WidthInBytes();

    CudaCtxPush ctxPush(cu_ctx);
    cuMemcpy2DAsync(&m, cu_str);
    cuStreamSynchronize(cu_str);

    return pOutput;
  }
};

struct rbg8_y final : public NppConvertSurface_Impl {
  rbg8_y(uint32_t width, uint32_t height, CUcontext context, CUstream stream)
      : NppConvertSurface_Impl(context, stream, RGB, Y) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (SurfaceRGB*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    NppiSize roi = {(int)pOutput->Width(), (int)pOutput->Height()};

    auto ret = nppiRGBToGray_8u_C3C1R_Ctx(
        (const Npp8u*)pInput->PlanePtr(), pInput->Pitch(),
        (Npp8u*)pOutput->PlanePtr(), pOutput->Pitch(), roi, nppCtx);

    return pOutput;
  }
};

struct yuv420_rgb final : public NppConvertSurface_Impl {
  yuv420_rgb(uint32_t width, uint32_t height, CUcontext context,
             CUstream stream)
      : NppConvertSurface_Impl(context, stream, YUV420, RGB) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto const params = GetParams(pCtx);
    auto const color_space = std::get<0>(params);
    auto const color_range = std::get<1>(params);

    auto pInput = (SurfaceYUV420*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    const Npp8u* const pSrc[] = {(const Npp8u*)pInput->PlanePtr(0U),
                                 (const Npp8u*)pInput->PlanePtr(1U),
                                 (const Npp8u*)pInput->PlanePtr(2U)};
    Npp8u* pDst = (Npp8u*)pOutput->PlanePtr();
    int srcStep[] = {(int)pInput->Pitch(0U), (int)pInput->Pitch(1U),
                     (int)pInput->Pitch(2U)};
    int dstStep = (int)pOutput->Pitch();
    NppiSize roi = {(int)pOutput->Width(), (int)pOutput->Height()};
    CudaCtxPush ctxPush(cu_ctx);
    auto err = NPP_NO_ERROR;

    switch (color_space) {
    case BT_709:
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    case BT_601:
      if (JPEG == color_range) {
        err = nppiYUV420ToRGB_8u_P3C3R_Ctx(pSrc, srcStep, pDst, dstStep, roi,
                                           nppCtx);
      } else {
        err = nppiYCbCr420ToRGB_8u_P3C3R_Ctx(pSrc, srcStep, pDst, dstStep, roi,
                                             nppCtx);
      }
      break;
    default:
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    }

    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct yuv420_bgr final : public NppConvertSurface_Impl {
  yuv420_bgr(uint32_t width, uint32_t height, CUcontext context,
             CUstream stream)
      : NppConvertSurface_Impl(context, stream, YUV420, BGR) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto const params = GetParams(pCtx);
    auto const color_space = std::get<0>(params);
    auto const color_range = std::get<1>(params);

    auto pInput = (SurfaceYUV420*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    const Npp8u* const pSrc[] = {(const Npp8u*)pInput->PlanePtr(0U),
                                 (const Npp8u*)pInput->PlanePtr(1U),
                                 (const Npp8u*)pInput->PlanePtr(2U)};
    Npp8u* pDst = (Npp8u*)pOutput->PlanePtr();
    int srcStep[] = {(int)pInput->Pitch(0U), (int)pInput->Pitch(1U),
                     (int)pInput->Pitch(2U)};
    int dstStep = (int)pOutput->Pitch();
    NppiSize roi = {(int)pOutput->Width(), (int)pOutput->Height()};
    CudaCtxPush ctxPush(cu_ctx);
    auto err = NPP_NO_ERROR;

    switch (color_space) {
    case BT_709:
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    case BT_601:
      if (JPEG == color_range) {
        err = nppiYUV420ToBGR_8u_P3C3R_Ctx(pSrc, srcStep, pDst, dstStep, roi,
                                           nppCtx);
      } else {
        err = nppiYCbCr420ToBGR_8u_P3C3R_Ctx(pSrc, srcStep, pDst, dstStep, roi,
                                             nppCtx);
      }
      break;
    default:
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    }

    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct yuv444_bgr final : public NppConvertSurface_Impl {
  yuv444_bgr(uint32_t width, uint32_t height, CUcontext context,
             CUstream stream)
      : NppConvertSurface_Impl(context, stream, YUV444, BGR) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto const params = GetParams(pCtx);
    auto const color_space = std::get<0>(params);
    auto const color_range = std::get<1>(params);

    if (BT_601 != color_space) {
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    }

    auto pInput = (SurfaceYUV444*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    const Npp8u* const pSrc[] = {(const Npp8u*)pInput->PlanePtr(0U),
                                 (const Npp8u*)pInput->PlanePtr(1U),
                                 (const Npp8u*)pInput->PlanePtr(2U)};
    Npp8u* pDst = (Npp8u*)pOutput->PlanePtr();
    int srcStep = (int)pInput->Pitch();
    int dstStep = (int)pOutput->Pitch();
    NppiSize roi = {(int)pOutput->Width(), (int)pOutput->Height()};
    CudaCtxPush ctxPush(cu_ctx);
    auto err = NPP_NO_ERROR;

    switch (color_range) {
    case MPEG:
      err = nppiYCbCrToBGR_8u_P3C3R_Ctx(pSrc, srcStep, pDst, dstStep, roi,
                                        nppCtx);
      break;
    case JPEG:
      err =
          nppiYUVToBGR_8u_P3C3R_Ctx(pSrc, srcStep, pDst, dstStep, roi, nppCtx);
      break;
    default:
      err = NPP_NO_OPERATION_WARNING;
      break;
    }

    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct yuv444_rgb final : public NppConvertSurface_Impl {
  yuv444_rgb(uint32_t width, uint32_t height, CUcontext context,
             CUstream stream)
      : NppConvertSurface_Impl(context, stream, YUV444, RGB) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto const params = GetParams(pCtx);
    auto const color_space = std::get<0>(params);
    auto const color_range = std::get<1>(params);

    if (BT_601 != color_space) {
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    }

    auto pInput = (SurfaceYUV444*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    const Npp8u* const pSrc[] = {(const Npp8u*)pInput->PlanePtr(0U),
                                 (const Npp8u*)pInput->PlanePtr(1U),
                                 (const Npp8u*)pInput->PlanePtr(2U)};
    Npp8u* pDst = (Npp8u*)pOutput->PlanePtr();
    int srcStep = (int)pInput->Pitch();
    int dstStep = (int)pOutput->Pitch();
    NppiSize roi = {(int)pOutput->Width(), (int)pOutput->Height()};
    CudaCtxPush ctxPush(cu_ctx);
    auto err = NPP_NO_ERROR;

    switch (color_range) {
    case JPEG:
      err =
          nppiYUVToRGB_8u_P3C3R_Ctx(pSrc, srcStep, pDst, dstStep, roi, nppCtx);
      break;
    default:
      err = NPP_NO_OPERATION_WARNING;
      break;
    }

    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct yuv444_rgb_planar final : public NppConvertSurface_Impl {
  yuv444_rgb_planar(uint32_t width, uint32_t height, CUcontext context,
                    CUstream stream)
      : NppConvertSurface_Impl(context, stream, YUV444, RGB_PLANAR) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto const params = GetParams(pCtx);
    auto const color_space = std::get<0>(params);
    auto const color_range = std::get<1>(params);

    if (BT_601 != color_space) {
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    }

    auto pInput = (SurfaceYUV444*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    const Npp8u* const pSrc[] = {(const Npp8u*)pInput->PlanePtr(0U),
                                 (const Npp8u*)pInput->PlanePtr(1U),
                                 (const Npp8u*)pInput->PlanePtr(2U)};
    Npp8u* pDst[] = {(Npp8u*)pOutput->PlanePtr(0), (Npp8u*)pOutput->PlanePtr(1),
                     (Npp8u*)pOutput->PlanePtr(2)};
    int srcStep = (int)pInput->Pitch();
    int dstStep = (int)pOutput->Pitch();
    NppiSize roi = {(int)pOutput->Width(), (int)pOutput->Height()};
    CudaCtxPush ctxPush(cu_ctx);
    auto err = NPP_NO_ERROR;

    switch (color_range) {
    case JPEG:
      err = nppiYUVToRGB_8u_P3R_Ctx(pSrc, srcStep, pDst, dstStep, roi, nppCtx);
      break;
    default:
      err = NPP_NO_OPERATION_WARNING;
      break;
    }

    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct bgr_yuv444 final : public NppConvertSurface_Impl {
  bgr_yuv444(uint32_t width, uint32_t height, CUcontext context,
             CUstream stream)
      : NppConvertSurface_Impl(context, stream, BGR, YUV444) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto const params = GetParams(pCtx);
    auto const color_space = std::get<0>(params);
    auto const color_range = std::get<1>(params);

    if (BT_601 != color_space) {
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    }

    auto pInput = (SurfaceBGR*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    const Npp8u* pSrc = (const Npp8u*)pInput->PlanePtr();
    Npp8u* pDst[] = {(Npp8u*)pOutput->PlanePtr(0U),
                     (Npp8u*)pOutput->PlanePtr(1U),
                     (Npp8u*)pOutput->PlanePtr(2U)};
    int srcStep = (int)pInput->Pitch();
    int dstStep = (int)pOutput->Pitch();
    NppiSize roi = {(int)pOutput->Width(), (int)pOutput->Height()};
    CudaCtxPush ctxPush(cu_ctx);
    auto err = NPP_NO_ERROR;

    switch (color_range) {
    case MPEG:
      err = nppiBGRToYCbCr_8u_C3P3R_Ctx(pSrc, srcStep, pDst, dstStep, roi,
                                        nppCtx);
      break;
    case JPEG:
      err =
          nppiBGRToYUV_8u_C3P3R_Ctx(pSrc, srcStep, pDst, dstStep, roi, nppCtx);
      break;
    default:
      err = NPP_NO_OPERATION_WARNING;
      break;
    }

    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct rgb_yuv444 final : public NppConvertSurface_Impl {
  rgb_yuv444(uint32_t width, uint32_t height, CUcontext context,
             CUstream stream)
      : NppConvertSurface_Impl(context, stream, RGB, YUV444) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (SurfaceRGB*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    auto const params = GetParams(pCtx);
    auto const color_space = std::get<0>(params);
    auto const color_range = std::get<1>(params);

    if (BT_601 != color_space) {
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    }

    const Npp8u* pSrc = (const Npp8u*)pInput->PlanePtr();
    int srcStep = pInput->Pitch();
    Npp8u* pDst[] = {(Npp8u*)pOutput->PlanePtr(0U),
                     (Npp8u*)pOutput->PlanePtr(1U),
                     (Npp8u*)pOutput->PlanePtr(2U)};
    int dstStep = pOutput->Pitch();
    NppiSize roi = {(int)pOutput->Width(), (int)pOutput->Height()};

    CudaCtxPush ctxPush(cu_ctx);
    auto err = NPP_NO_ERROR;
    switch (color_range) {
    case JPEG:
      err =
          nppiRGBToYUV_8u_C3P3R_Ctx(pSrc, srcStep, pDst, dstStep, roi, nppCtx);
      break;
    case MPEG:
      err = nppiRGBToYCbCr_8u_C3R_Ctx(pSrc, srcStep, pDst[0], dstStep, roi,
                                      nppCtx);
      break;
    default:
      err = NPP_NO_OPERATION_WARNING;
    }

    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct rgb_planar_yuv444 final : public NppConvertSurface_Impl {
  rgb_planar_yuv444(uint32_t width, uint32_t height, CUcontext context,
                    CUstream stream)
      : NppConvertSurface_Impl(context, stream, RGB_PLANAR, YUV444) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (SurfaceRGBPlanar*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    auto const params = GetParams(pCtx);
    auto const color_space = std::get<0>(params);
    auto const color_range = std::get<1>(params);

    if (BT_601 != color_space) {
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    }

    const Npp8u* pSrc[] = {(const Npp8u*)pInput->PlanePtr(0U),
                           (const Npp8u*)pInput->PlanePtr(1U),
                           (const Npp8u*)pInput->PlanePtr(2U)};
    int srcStep = pInput->Pitch();
    Npp8u* pDst[] = {(Npp8u*)pOutput->PlanePtr(0U),
                     (Npp8u*)pOutput->PlanePtr(1U),
                     (Npp8u*)pOutput->PlanePtr(2U)};
    int dstStep = pOutput->Pitch();
    NppiSize roi = {(int)pOutput->Width(), (int)pOutput->Height()};

    CudaCtxPush ctxPush(cu_ctx);
    auto err = NPP_NO_ERROR;
    switch (color_range) {
    case JPEG:
      err = nppiRGBToYUV_8u_P3R_Ctx(pSrc, srcStep, pDst, dstStep, roi, nppCtx);
      break;
    case MPEG:
      err =
          nppiRGBToYCbCr_8u_P3R_Ctx(pSrc, srcStep, pDst, dstStep, roi, nppCtx);
      break;
    default:
      err = NPP_NO_OPERATION_WARNING;
      break;
    }

    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct y_yuv444 final : public NppConvertSurface_Impl {
  y_yuv444(uint32_t width, uint32_t height, CUcontext context, CUstream stream)
      : NppConvertSurface_Impl(context, stream, Y, YUV444) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (SurfaceY*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    // Make gray U and V channels;
    for (int i = 1; i < pOutput->NumPlanes(); i++) {
      const Npp8u nValue = 128U;
      Npp8u* pDst = (Npp8u*)pOutput->PlanePtr(i);
      int nDstStep = pOutput->Pitch(i);
      NppiSize roi = {(int)pOutput->Width(i), (int)pOutput->Height(i)};
      auto err = nppiSet_8u_C1R_Ctx(nValue, pDst, nDstStep, roi, nppCtx);
      if (NPP_NO_ERROR != err) {
        details.info = TaskExecInfo::FAIL;
        return nullptr;
      }
    }

    // Copy Y channel;
    const Npp8u* pSrc = (const Npp8u*)pInput->PlanePtr();
    int nSrcStep = pInput->Pitch();
    Npp8u* pDst = (Npp8u*)pOutput->PlanePtr(0U);
    int nDstStep = pOutput->Pitch(0U);
    NppiSize roi = {(int)pInput->Width(), (int)pInput->Height()};
    auto err = nppiCopy_8u_C1R_Ctx(pSrc, nSrcStep, pDst, nDstStep, roi, nppCtx);
    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct rgb_yuv420 final : public NppConvertSurface_Impl {
  rgb_yuv420(uint32_t width, uint32_t height, CUcontext context,
             CUstream stream)
      : NppConvertSurface_Impl(context, stream, RGB, YUV420) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (SurfaceRGB*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    auto const params = GetParams(pCtx);
    auto const color_space = std::get<0>(params);
    auto const color_range = std::get<1>(params);

    if (BT_601 != color_space) {
      details.info = TaskExecInfo::UNSUPPORTED_FMT_CONV_PARAMS;
      return nullptr;
    }

    const Npp8u* pSrc = (const Npp8u*)pInput->PlanePtr();
    int srcStep = pInput->Pitch();
    Npp8u* pDst[] = {(Npp8u*)pOutput->PlanePtr(0U),
                     (Npp8u*)pOutput->PlanePtr(1U),
                     (Npp8u*)pOutput->PlanePtr(2U)};
    int dstStep[] = {(int)pOutput->Pitch(0U), (int)pOutput->Pitch(1U),
                     (int)pOutput->Pitch(2U)};
    NppiSize roi = {(int)pOutput->Width(), (int)pOutput->Height()};

    CudaCtxPush ctxPush(cu_ctx);
    auto err = NPP_NO_ERROR;
    switch (color_range) {
    case JPEG:
      err = nppiRGBToYUV420_8u_C3P3R_Ctx(pSrc, srcStep, pDst, dstStep, roi,
                                         nppCtx);
      break;

    case MPEG:
      err = nppiRGBToYCbCr420_8u_C3P3R_Ctx(pSrc, srcStep, pDst, dstStep, roi,
                                           nppCtx);
      break;

    default:
      err = NPP_NO_OPERATION_WARNING;
      break;
    }

    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct yuv420_nv12 final : public NppConvertSurface_Impl {
  yuv420_nv12(uint32_t width, uint32_t height, CUcontext context,
              CUstream stream)
      : NppConvertSurface_Impl(context, stream, YUV420, NV12) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (Surface*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    const Npp8u* const pSrc[] = {(const Npp8u*)pInput->PlanePtr(0U),
                                 (const Npp8u*)pInput->PlanePtr(1U),
                                 (const Npp8u*)pInput->PlanePtr(2U)};

    Npp8u* pDst[] = {(Npp8u*)pOutput->PlanePtr(0U),
                     (Npp8u*)pOutput->PlanePtr(1U)};

    int srcStep[] = {(int)pInput->Pitch(0U), (int)pInput->Pitch(1U),
                     (int)pInput->Pitch(2U)};
    int dstStep[] = {(int)pOutput->Pitch(0U), (int)pOutput->Pitch(1U)};
    NppiSize roi = {(int)pInput->Width(), (int)pInput->Height()};

    CudaCtxPush ctxPush(cu_ctx);
    auto err = nppiYCbCr420_8u_P3P2R_Ctx(pSrc, srcStep, pDst[0], dstStep[0],
                                         pDst[1], dstStep[1], roi, nppCtx);
    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct rgb8_deinterleave final : public NppConvertSurface_Impl {
  rgb8_deinterleave(uint32_t width, uint32_t height, CUcontext context,
                    CUstream stream)
      : NppConvertSurface_Impl(context, stream, RGB, RGB_PLANAR) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (SurfaceRGB*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    const Npp8u* pSrc = (const Npp8u*)pInput->PlanePtr();
    int nSrcStep = pInput->Pitch();
    Npp8u* aDst[] = {
        (Npp8u*)pOutput->PlanePtr(),
        (Npp8u*)pOutput->PlanePtr() + pOutput->Height() * pOutput->Pitch(),
        (Npp8u*)pOutput->PlanePtr() + pOutput->Height() * pOutput->Pitch() * 2};
    int nDstStep = pOutput->Pitch();
    NppiSize oSizeRoi = {0};
    oSizeRoi.height = pOutput->Height();
    oSizeRoi.width = pOutput->Width();

    CudaCtxPush ctxPush(cu_ctx);
    auto err =
        nppiCopy_8u_C3P3R_Ctx(pSrc, nSrcStep, aDst, nDstStep, oSizeRoi, nppCtx);
    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct rgb8_interleave final : public NppConvertSurface_Impl {
  rgb8_interleave(uint32_t width, uint32_t height, CUcontext context,
                  CUstream stream)
      : NppConvertSurface_Impl(context, stream, RGB_PLANAR, RGB) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (SurfaceRGBPlanar*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    const Npp8u* const pSrc[] = {
        (Npp8u*)pInput->PlanePtr(),
        (Npp8u*)pInput->PlanePtr() + pInput->Height() * pInput->Pitch(),
        (Npp8u*)pInput->PlanePtr() + pInput->Height() * pInput->Pitch() * 2};
    int nSrcStep = pInput->Pitch();
    Npp8u* pDst = (Npp8u*)pOutput->PlanePtr();
    int nDstStep = pOutput->Pitch();
    NppiSize oSizeRoi = {0};
    oSizeRoi.height = pOutput->Height();
    oSizeRoi.width = pOutput->Width();

    CudaCtxPush ctxPush(cu_ctx);
    auto err =
        nppiCopy_8u_P3C3R_Ctx(pSrc, nSrcStep, pDst, nDstStep, oSizeRoi, nppCtx);
    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct rgb_bgr final : public NppConvertSurface_Impl {
  rgb_bgr(uint32_t width, uint32_t height, CUcontext context, CUstream stream)
      : NppConvertSurface_Impl(context, stream, RGB, BGR) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (SurfaceRGB*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    const Npp8u* pSrc = (const Npp8u*)pInput->PlanePtr();
    int nSrcStep = pInput->Pitch();
    Npp8u* pDst = (Npp8u*)pOutput->PlanePtr();
    int nDstStep = pOutput->Pitch();
    NppiSize oSizeRoi = {0};
    oSizeRoi.height = pOutput->Height();
    oSizeRoi.width = pOutput->Width();
    // rgb to brg
    const int aDstOrder[3] = {2, 1, 0};
    CudaCtxPush ctxPush(cu_ctx);
    auto err = nppiSwapChannels_8u_C3R_Ctx(pSrc, nSrcStep, pDst, nDstStep,
                                           oSizeRoi, aDstOrder, nppCtx);
    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct bgr_rgb final : public NppConvertSurface_Impl {
  bgr_rgb(uint32_t width, uint32_t height, CUcontext context, CUstream stream)
      : NppConvertSurface_Impl(context, stream, BGR, RGB) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (SurfaceBGR*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    const Npp8u* pSrc = (const Npp8u*)pInput->PlanePtr();
    int nSrcStep = pInput->Pitch();
    Npp8u* pDst = (Npp8u*)pOutput->PlanePtr();
    int nDstStep = pOutput->Pitch();
    NppiSize oSizeRoi = {0};
    oSizeRoi.height = pOutput->Height();
    oSizeRoi.width = pOutput->Width();
    // brg to rgb
    const int aDstOrder[3] = {2, 1, 0};
    CudaCtxPush ctxPush(cu_ctx);
    auto err = nppiSwapChannels_8u_C3R_Ctx(pSrc, nSrcStep, pDst, nDstStep,
                                           oSizeRoi, aDstOrder, nppCtx);
    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct rbg8_rgb32f final : public NppConvertSurface_Impl {
  rbg8_rgb32f(uint32_t width, uint32_t height, CUcontext context,
              CUstream stream)
      : NppConvertSurface_Impl(context, stream, RGB, RGB_32F) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (SurfaceRGB*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    const Npp8u* pSrc = (const Npp8u*)pInput->PlanePtr();

    int nSrcStep = pInput->Pitch();
    Npp32f* pDst = (Npp32f*)pOutput->PlanePtr();
    int nDstStep = pOutput->Pitch();
    NppiSize oSizeRoi = {0};
    oSizeRoi.height = pOutput->Height();
    oSizeRoi.width = pOutput->Width();
    Npp32f nMin = 0.0;
    Npp32f nMax = 1.0;
    const int aDstOrder[3] = {2, 1, 0};

    CudaCtxPush ctxPush(cu_ctx);

    auto err = nppiScale_8u32f_C3R_Ctx(pSrc, nSrcStep, pDst, nDstStep, oSizeRoi,
                                       nMin, nMax, nppCtx);
    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

struct rgb32f_deinterleave final : public NppConvertSurface_Impl {
  rgb32f_deinterleave(uint32_t width, uint32_t height, CUcontext context,
                      CUstream stream)
      : NppConvertSurface_Impl(context, stream, RGB_32F, RGB_32F_PLANAR) {}

  Token* Execute(Token* pSrcToken, Token* pDstToken,
                 ColorspaceConversionContext* pCtx) override {
    NvtxMark tick(GetNvtxTickName().c_str());
    details.info = TaskExecInfo::SUCCESS;

    auto pInput = (SurfaceRGB*)pSrcToken;
    auto pOutput = GetOutput(pInput, (Surface*)pDstToken);

    if (!Validate(pInput, pOutput)) {
      details.info = TaskExecInfo::INVALID_INPUT;
      return nullptr;
    }

    const Npp32f* pSrc = (const Npp32f*)pInput->PlanePtr();
    int nSrcStep = pInput->Pitch();
    Npp32f* aDst[] = {(Npp32f*)((uint8_t*)pOutput->PlanePtr()),
                      (Npp32f*)((uint8_t*)pOutput->PlanePtr() +
                                pOutput->Height() * pOutput->Pitch()),
                      (Npp32f*)((uint8_t*)pOutput->PlanePtr() +
                                pOutput->Height() * pOutput->Pitch() * 2)};
    int nDstStep = pOutput->Pitch();
    NppiSize oSizeRoi = {0};
    oSizeRoi.height = pOutput->Height();
    oSizeRoi.width = pOutput->Width();

    CudaCtxPush ctxPush(cu_ctx);
    auto err = nppiCopy_32f_C3P3R_Ctx(pSrc, nSrcStep, aDst, nDstStep, oSizeRoi,
                                      nppCtx);
    if (NPP_NO_ERROR != err) {
      details.info = TaskExecInfo::FAIL;
      return nullptr;
    }

    return pOutput;
  }
};

} // namespace VPF

auto const cuda_stream_sync = [](void* stream) {
  cuStreamSynchronize((CUstream)stream);
};

ConvertSurface::ConvertSurface(uint32_t width, uint32_t height,
                               Pixel_Format inFormat, Pixel_Format outFormat,
                               CUcontext ctx, CUstream str)
    : Task("NppConvertSurface", ConvertSurface::numInputs,
           ConvertSurface::numOutputs, nullptr, nullptr) {
  if (NV12 == inFormat && YUV420 == outFormat) {
    pImpl = new nv12_yuv420(width, height, ctx, str);
  } else if (YUV420 == inFormat && NV12 == outFormat) {
    pImpl = new yuv420_nv12(width, height, ctx, str);
  } else if (NV12 == inFormat && RGB == outFormat) {
    pImpl = new nv12_rgb(width, height, ctx, str);
  } else if (NV12 == inFormat && BGR == outFormat) {
    pImpl = new nv12_bgr(width, height, ctx, str);
  } else if (RGB == inFormat && RGB_PLANAR == outFormat) {
    pImpl = new rgb8_deinterleave(width, height, ctx, str);
  } else if (RGB_PLANAR == inFormat && RGB == outFormat) {
    pImpl = new rgb8_interleave(width, height, ctx, str);
  } else if (RGB_PLANAR == inFormat && YUV444 == outFormat) {
    pImpl = new rgb_planar_yuv444(width, height, ctx, str);
  } else if (Y == inFormat && YUV444 == outFormat) {
    pImpl = new y_yuv444(width, height, ctx, str);
  } else if (YUV420 == inFormat && RGB == outFormat) {
    pImpl = new yuv420_rgb(width, height, ctx, str);
  } else if (RGB == inFormat && YUV420 == outFormat) {
    pImpl = new rgb_yuv420(width, height, ctx, str);
  } else if (RGB == inFormat && YUV444 == outFormat) {
    pImpl = new rgb_yuv444(width, height, ctx, str);
  } else if (RGB == inFormat && BGR == outFormat) {
    pImpl = new rgb_bgr(width, height, ctx, str);
  } else if (BGR == inFormat && RGB == outFormat) {
    pImpl = new bgr_rgb(width, height, ctx, str);
  } else if (YUV420 == inFormat && BGR == outFormat) {
    pImpl = new yuv420_bgr(width, height, ctx, str);
  } else if (YUV444 == inFormat && BGR == outFormat) {
    pImpl = new yuv444_bgr(width, height, ctx, str);
  } else if (YUV444 == inFormat && RGB == outFormat) {
    pImpl = new yuv444_rgb(width, height, ctx, str);
  } else if (BGR == inFormat && YUV444 == outFormat) {
    pImpl = new bgr_yuv444(width, height, ctx, str);
  } else if (NV12 == inFormat && Y == outFormat) {
    pImpl = new nv12_y(width, height, ctx, str);
  } else if (RGB == inFormat && RGB_32F == outFormat) {
    pImpl = new rbg8_rgb32f(width, height, ctx, str);
  } else if (RGB == inFormat && Y == outFormat) {
    pImpl = new rbg8_y(width, height, ctx, str);
  } else if (RGB_32F == inFormat && RGB_32F_PLANAR == outFormat) {
    pImpl = new rgb32f_deinterleave(width, height, ctx, str);
  } else {
    throw std::invalid_argument("Unsupported pixel format conversion");
  }
}

ConvertSurface::~ConvertSurface() { delete pImpl; }

ConvertSurface* ConvertSurface::Make(uint32_t width, uint32_t height,
                                     Pixel_Format inFormat,
                                     Pixel_Format outFormat, CUcontext ctx,
                                     CUstream str) {
  return new ConvertSurface(width, height, inFormat, outFormat, ctx, str);
}

TaskExecStatus ConvertSurface::Run() {
  ClearOutputs();

  ColorspaceConversionContext* pCtx = nullptr;
  auto ctx_buf = (Buffer*)GetInput(2U);
  if (ctx_buf) {
    pCtx = ctx_buf->GetDataAs<ColorspaceConversionContext>();
  }

  auto pOutput = pImpl->Execute(GetInput(0), GetInput(1), pCtx);
  pImpl->pDetails->CopyFrom(sizeof(pImpl->details),
                            (const void*)&pImpl->details);

  SetOutput(pOutput, 0U);
  SetOutput(pImpl->pDetails.get(), 1U);

  return TASK_EXEC_SUCCESS;
}