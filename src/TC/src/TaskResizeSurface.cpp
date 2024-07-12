#include "MemoryInterfaces.hpp"
#include "NppCommon.hpp"
#include "Tasks.hpp"

#include <stdexcept>

namespace VPF {

static const TaskExecDetails s_invalid_src_dst(TaskExecStatus::TASK_EXEC_FAIL,
                                               TaskExecInfo::INVALID_INPUT,
                                               "invalid src / dst");

static const TaskExecDetails s_success(TaskExecStatus::TASK_EXEC_SUCCESS,
                                       TaskExecInfo::SUCCESS);

static const TaskExecDetails s_fail(TaskExecStatus::TASK_EXEC_FAIL,
                                    TaskExecInfo::FAIL);

struct ResizeSurface_Impl {
  CUstream cu_str;
  NppStreamContext nppCtx;

  ResizeSurface_Impl(Pixel_Format format, CUstream str) : cu_str(str) {
    SetupNppContext(cu_str, nppCtx);
  }

  virtual ~ResizeSurface_Impl() = default;

  virtual TaskExecDetails Run(Surface& src, Surface& dst) = 0;
};

struct NppResizeSurfacePacked3C_Impl final : ResizeSurface_Impl {
  NppResizeSurfacePacked3C_Impl(CUstream str, Pixel_Format format)
      : ResizeSurface_Impl(format, str) {}

  ~NppResizeSurfacePacked3C_Impl() = default;

  TaskExecDetails Run(Surface& src, Surface& dst) {
    NvtxMark tick("NppResizeSurfacePacked3C");

    if (dst.PixelFormat() != src.PixelFormat()) {
      return s_invalid_src_dst;
    }

    auto srcPlane = src.GetSurfacePlane();
    auto dstPlane = dst.GetSurfacePlane();

    const Npp8u* pSrc = (const Npp8u*)srcPlane.GpuMem();
    int nSrcStep = (int)src.Pitch();
    NppiSize oSrcSize = {0};
    oSrcSize.width = src.Width();
    oSrcSize.height = src.Height();
    NppiRect oSrcRectROI = {0};
    oSrcRectROI.width = oSrcSize.width;
    oSrcRectROI.height = oSrcSize.height;

    Npp8u* pDst = (Npp8u*)dstPlane.GpuMem();
    int nDstStep = (int)dst.Pitch();
    NppiSize oDstSize = {0};
    oDstSize.width = dst.Width();
    oDstSize.height = dst.Height();
    NppiRect oDstRectROI = {0};
    oDstRectROI.width = oDstSize.width;
    oDstRectROI.height = oDstSize.height;
    int eInterpolation = NPPI_INTER_LANCZOS;

    CudaCtxPush ctxPush(cu_str);
    auto ret = nppiResize_8u_C3R_Ctx(pSrc, nSrcStep, oSrcSize, oSrcRectROI,
                                     pDst, nDstStep, oDstSize, oDstRectROI,
                                     eInterpolation, nppCtx);
    if (NPP_NO_ERROR != ret) {
      return s_fail;
    }

    return s_success;
  }
};

// Resize planar 8 bit surface (YUV420, YCbCr420);
struct NppResizeSurfacePlanar_Impl final : ResizeSurface_Impl {
  NppResizeSurfacePlanar_Impl(CUstream str, Pixel_Format format)
      : ResizeSurface_Impl(format, str) {}

  ~NppResizeSurfacePlanar_Impl() = default;

  TaskExecDetails Run(Surface& src, Surface& dst) {
    NvtxMark tick("NppResizeSurfacePlanar");

    if (dst.PixelFormat() != src.PixelFormat()) {
      return s_invalid_src_dst;
    }

    for (auto plane = 0; plane < dst.NumPlanes(); plane++) {
      auto srcPlane = src.GetSurfacePlane(plane);
      auto dstPlane = dst.GetSurfacePlane(plane);

      const Npp8u* pSrc = (const Npp8u*)srcPlane.GpuMem();
      int nSrcStep = (int)srcPlane.Pitch();
      NppiSize oSrcSize = {0};
      oSrcSize.width = srcPlane.Width();
      oSrcSize.height = srcPlane.Height();
      NppiRect oSrcRectROI = {0};
      oSrcRectROI.width = oSrcSize.width;
      oSrcRectROI.height = oSrcSize.height;

      Npp8u* pDst = (Npp8u*)dstPlane.GpuMem();
      int nDstStep = (int)dstPlane.Pitch();
      NppiSize oDstSize = {0};
      oDstSize.width = dstPlane.Width();
      oDstSize.height = dstPlane.Height();
      NppiRect oDstRectROI = {0};
      oDstRectROI.width = oDstSize.width;
      oDstRectROI.height = oDstSize.height;
      int eInterpolation = NPPI_INTER_LANCZOS;

      CudaCtxPush ctxPush(cu_str);
      auto ret = nppiResize_8u_C1R_Ctx(pSrc, nSrcStep, oSrcSize, oSrcRectROI,
                                       pDst, nDstStep, oDstSize, oDstRectROI,
                                       eInterpolation, nppCtx);
      if (NPP_NO_ERROR != ret) {
        return s_fail;
      }
    }

    return s_success;
  }
};

// Resize semiplanar 8 bit NV12 surface;
struct ResizeSurfaceSemiPlanar_Impl final : ResizeSurface_Impl {
  ResizeSurfaceSemiPlanar_Impl(CUstream str, Pixel_Format format)
      : ResizeSurface_Impl(format, str) {
    m_cvt_nv12_yuv420 = std::make_unique<ConvertSurface>(NV12, YUV420, str);
    m_cvt_yuv420_nv12 = std::make_unique<ConvertSurface>(YUV420, NV12, str);
    m_resizer = std::make_unique<ResizeSurface>(YUV420, str);
  }

  ~ResizeSurfaceSemiPlanar_Impl() = default;

  TaskExecDetails Run(Surface& src, Surface& dst) {
    NvtxMark tick("NppResizeSurfaceSemiPlanar");

    // Deal with temporary surfaces;
    if (!m_src_yuv420 || m_src_yuv420->Width() != src.Width() ||
        m_src_yuv420->Height() != src.Height()) {
      m_src_yuv420 = std::unique_ptr<Surface>(
          Surface::Make(YUV420, src.Width(), src.Height(), src.Context()));
    }

    if (!m_dst_yuv420 || m_dst_yuv420->Width() != dst.Width() ||
        m_dst_yuv420->Height() != dst.Height()) {
      m_dst_yuv420 = std::unique_ptr<Surface>(
          Surface::Make(YUV420, dst.Width(), dst.Height(), src.Context()));
    }

    // Convert from NV12 to YUV420;
    m_cvt_nv12_yuv420->SetInput((Token*)&src, 0U);
    m_cvt_nv12_yuv420->SetInput((Token*)m_src_yuv420.get(), 1U);
    if (TaskExecStatus::TASK_EXEC_SUCCESS !=
        m_cvt_nv12_yuv420->Execute().m_status) {
      return s_fail;
    }

    // Resize YUV420;
    m_resizer->SetInput((Token*)m_src_yuv420.get(), 0U);
    m_resizer->SetInput((Token*)m_dst_yuv420.get(), 1U);
    if (TaskExecStatus::TASK_EXEC_SUCCESS != m_resizer->Execute().m_status) {
      return s_fail;
    }

    // Convert back to NV12;
    m_cvt_yuv420_nv12->SetInput((Token*)m_dst_yuv420.get(), 0U);
    m_cvt_yuv420_nv12->SetInput((Token*)&dst, 1U);
    if (TaskExecStatus::TASK_EXEC_SUCCESS !=
        m_cvt_yuv420_nv12->Execute().m_status) {
      return s_fail;
    }

    return s_success;
  }

  /* NPP cant convert semi-planar surfaces, hence need to bash around with pixel
   * format conversion back and forth. Sic!
   */
  std::unique_ptr<ResizeSurface> m_resizer;

  std::unique_ptr<Surface> m_src_yuv420;
  std::unique_ptr<Surface> m_dst_yuv420;

  std::unique_ptr<ConvertSurface> m_cvt_nv12_yuv420;
  std::unique_ptr<ConvertSurface> m_cvt_yuv420_nv12;
};

struct NppResizeSurfacePacked32F3C_Impl final : ResizeSurface_Impl {
  NppResizeSurfacePacked32F3C_Impl(CUstream str, Pixel_Format format)
      : ResizeSurface_Impl(format, str) {}

  ~NppResizeSurfacePacked32F3C_Impl() = default;

  TaskExecDetails Run(Surface& src, Surface& dst) {
    NvtxMark tick("NppResizeSurfacePacked32F3C");

    if (dst.PixelFormat() != src.PixelFormat()) {
      return s_invalid_src_dst;
    }

    auto srcPlane = src.GetSurfacePlane();
    auto dstPlane = dst.GetSurfacePlane();

    const Npp32f* pSrc = (const Npp32f*)srcPlane.GpuMem();
    int nSrcStep = (int)src.Pitch();
    NppiSize oSrcSize = {0};
    oSrcSize.width = src.Width();
    oSrcSize.height = src.Height();
    NppiRect oSrcRectROI = {0};
    oSrcRectROI.width = oSrcSize.width;
    oSrcRectROI.height = oSrcSize.height;

    Npp32f* pDst = (Npp32f*)dstPlane.GpuMem();
    int nDstStep = (int)dst.Pitch();
    NppiSize oDstSize = {0};
    oDstSize.width = dst.Width();
    oDstSize.height = dst.Height();
    NppiRect oDstRectROI = {0};
    oDstRectROI.width = oDstSize.width;
    oDstRectROI.height = oDstSize.height;
    int eInterpolation = NPPI_INTER_LANCZOS;

    CudaCtxPush ctxPush(cu_str);
    auto ret = nppiResize_32f_C3R_Ctx(pSrc, nSrcStep, oSrcSize, oSrcRectROI,
                                      pDst, nDstStep, oDstSize, oDstRectROI,
                                      eInterpolation, nppCtx);
    if (NPP_NO_ERROR != ret) {
      return s_fail;
    }

    return s_success;
  }
};

// Resize planar 8 bit surface (YUV420, YCbCr420);
struct NppResizeSurface32FPlanar_Impl final : ResizeSurface_Impl {
  NppResizeSurface32FPlanar_Impl(CUstream str, Pixel_Format format)
      : ResizeSurface_Impl(format, str) {}

  ~NppResizeSurface32FPlanar_Impl() = default;

  TaskExecDetails Run(Surface& src, Surface& dst) {
    NvtxMark tick("NppResizeSurface32FPlanar");

    if (dst.PixelFormat() != src.PixelFormat()) {
      return s_invalid_src_dst;
    }

    for (auto plane = 0; plane < dst.NumPlanes(); plane++) {
      auto srcPlane = src.GetSurfacePlane(plane);
      auto dstPlane = dst.GetSurfacePlane(plane);

      const Npp32f* pSrc = (const Npp32f*)srcPlane.GpuMem();
      int nSrcStep = (int)srcPlane.Pitch();
      NppiSize oSrcSize = {0};
      oSrcSize.width = srcPlane.Width();
      oSrcSize.height = srcPlane.Height();
      NppiRect oSrcRectROI = {0};
      oSrcRectROI.width = oSrcSize.width;
      oSrcRectROI.height = oSrcSize.height;

      Npp32f* pDst = (Npp32f*)dstPlane.GpuMem();
      int nDstStep = (int)dstPlane.Pitch();
      NppiSize oDstSize = {0};
      oDstSize.width = dstPlane.Width();
      oDstSize.height = dstPlane.Height();
      NppiRect oDstRectROI = {0};
      oDstRectROI.width = oDstSize.width;
      oDstRectROI.height = oDstSize.height;
      int eInterpolation = NPPI_INTER_LANCZOS;

      CudaCtxPush ctxPush(cu_str);
      auto ret = nppiResize_32f_C1R_Ctx(pSrc, nSrcStep, oSrcSize, oSrcRectROI,
                                        pDst, nDstStep, oDstSize, oDstRectROI,
                                        eInterpolation, nppCtx);
      if (NPP_NO_ERROR != ret) {
        return s_fail;
      }
    }

    return s_success;
  }
};
} // namespace VPF

auto const cuda_stream_sync = [](void* stream) {
  cuStreamSynchronize((CUstream)stream);
};

ResizeSurface::ResizeSurface(Pixel_Format format, CUstream str)
    : Task("NppResizeSurface", ResizeSurface::numInputs,
           ResizeSurface::numOutputs, cuda_stream_sync, (void*)str) {
  if (RGB == format || BGR == format) {
    pImpl = new NppResizeSurfacePacked3C_Impl(str, format);
  } else if (YUV420 == format || YUV444 == format || RGB_PLANAR == format) {
    pImpl = new NppResizeSurfacePlanar_Impl(str, format);
  } else if (RGB_32F == format) {
    pImpl = new NppResizeSurfacePacked32F3C_Impl(str, format);
  } else if (RGB_32F_PLANAR == format) {
    pImpl = new NppResizeSurface32FPlanar_Impl(str, format);
  } else if (NV12 == format) {
    pImpl = new ResizeSurfaceSemiPlanar_Impl(str, format);
  } else {
    throw std::runtime_error("pixel format not supported");
  }
}

ResizeSurface::~ResizeSurface() { delete pImpl; }

TaskExecDetails ResizeSurface::Run() {
  NvtxMark tick(GetName());
  ClearOutputs();

  auto pInputSurface = (Surface*)GetInput(0U);
  if (!pInputSurface) {
    return s_invalid_src_dst;
  }

  auto pOutputSurface = (Surface*)GetInput(1U);
  if (!pOutputSurface) {
    return s_invalid_src_dst;
  }

  return pImpl->Run(*pInputSurface, *pOutputSurface);
}