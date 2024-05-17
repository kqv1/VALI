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

#pragma once
#include "CodecsSupport.hpp"
#include "MemoryInterfaces.hpp"
#include "NvCodecCLIOptions.h"
#include "TC_CORE.hpp"
#include "cuviddec.h"
#include "nvcuvid.h"
#include "tc_core_export.h" // generated by cmake

extern "C" {
#include <libavutil/frame.h>
}

#ifdef USE_NVTX
#include <nvtx3/nvToolsExt.h>
#define NVTX_PUSH(FNAME)                                                       \
  do {                                                                         \
    nvtxRangePush(FNAME);                                                      \
  } while (0);
#define NVTX_POP                                                               \
  do {                                                                         \
    nvtxRangePop();                                                            \
  } while (0);
#else
#define NVTX_PUSH(FNAME)
#define NVTX_POP
#endif

using namespace VPF;

// VPF stands for Video Processing Framework;
namespace VPF
{
class TC_CORE_EXPORT NvtxMark {
public:
  NvtxMark() = delete;
  NvtxMark(const NvtxMark& other) = delete;
  NvtxMark(const NvtxMark&& other) = delete;
  NvtxMark& operator=(const NvtxMark& other) = delete;
  NvtxMark(const char* fname) { NVTX_PUSH(fname) }
  ~NvtxMark() { NVTX_POP }
};

class TC_CORE_EXPORT NvencEncodeFrame final : public Task
{
public:
  NvencEncodeFrame() = delete;
  NvencEncodeFrame(const NvencEncodeFrame& other) = delete;
  NvencEncodeFrame& operator=(const NvencEncodeFrame& other) = delete;
  uint32_t GetWidth() const;
  uint32_t GetHeight() const;
  int GetCapability(NV_ENC_CAPS cap) const;

  TaskExecStatus Run() final;
  ~NvencEncodeFrame() final;
  static NvencEncodeFrame* Make(CUstream cuStream, CUcontext cuContext,
                                NvEncoderClInterface& cli_iface,
                                NV_ENC_BUFFER_FORMAT format, uint32_t width,
                                uint32_t height, bool verbose);

  bool Reconfigure(NvEncoderClInterface& cli_iface, bool force_idr,
                   bool reset_enc, bool verbose);

private:
  NvencEncodeFrame(CUstream cuStream, CUcontext cuContext,
                   NvEncoderClInterface& cli_iface, NV_ENC_BUFFER_FORMAT format,
                   uint32_t width, uint32_t height, bool verbose);
  static const uint32_t numInputs = 3U;
  static const uint32_t numOutputs = 1U;
  struct NvencEncodeFrame_Impl* pImpl = nullptr;
};

enum NV_DEC_CAPS {
  BIT_DEPTH_MINUS_8,
  IS_CODEC_SUPPORTED,
  OUTPUT_FORMAT_MASK,
  MAX_WIDTH,
  MAX_HEIGHT,
  MAX_MB_COUNT,
  MIN_WIDTH,
  MIN_HEIGHT,
#if CHECK_API_VERSION(11, 0)
  IS_HIST_SUPPORTED,
  HIST_COUNT_BIT_DEPTH,
  HIST_COUNT_BINS,
#endif
  NV_DEC_CAPS_NUM_ENTRIES
};

class TC_CORE_EXPORT NvdecDecodeFrame final : public Task
{
public:
  NvdecDecodeFrame() = delete;
  NvdecDecodeFrame(const NvdecDecodeFrame& other) = delete;
  NvdecDecodeFrame& operator=(const NvdecDecodeFrame& other) = delete;

  void GetDecodedFrameParams(uint32_t& width, uint32_t& height,
                             uint32_t& elemSize);

  int GetCapability(NV_DEC_CAPS cap) const;

  TaskExecStatus Run() final;
  uint32_t GetDeviceFramePitch();
  ~NvdecDecodeFrame() final;
  static NvdecDecodeFrame* Make(CUstream cuStream, CUcontext cuContext,
                                cudaVideoCodec videoCodec,
                                uint32_t decodedFramesPoolSize,
                                uint32_t coded_width, uint32_t coded_height,
                                Pixel_Format format);

private:
  static const uint32_t numInputs = 3U;
  static const uint32_t numOutputs = 3U;
  struct NvdecDecodeFrame_Impl* pImpl = nullptr;

  NvdecDecodeFrame(CUstream cuStream, CUcontext cuContext,
                   cudaVideoCodec videoCodec, uint32_t decodedFramesPoolSize,
                   uint32_t coded_width, uint32_t coded_height,
                   Pixel_Format format);
};

class TC_CORE_EXPORT FfmpegDecodeFrame final : public Task
{
public:
  FfmpegDecodeFrame() = delete;
  FfmpegDecodeFrame(const FfmpegDecodeFrame& other) = delete;
  FfmpegDecodeFrame& operator=(const FfmpegDecodeFrame& other) = delete;

  TaskExecStatus Run() final;
  TaskExecStatus GetSideData(AVFrameSideDataType);
  void GetParams(MuxingParams& params);

  ~FfmpegDecodeFrame() final;
  static FfmpegDecodeFrame* Make(const char* URL,
                                 NvDecoderClInterface& cli_iface);
  const PacketData& GetLastPacketData() const;

private:
  static const uint32_t num_inputs = 0U;
  // Reconstructed pixels + side data;
  static const uint32_t num_outputs = 2U;
  struct FfmpegDecodeFrame_Impl* pImpl = nullptr;

  FfmpegDecodeFrame(const char* URL, NvDecoderClInterface& cli_iface);
};

class TC_CORE_EXPORT CudaUploadFrame final : public Task {
public:
  CudaUploadFrame() = delete;
  CudaUploadFrame(const CudaUploadFrame& other) = delete;
  CudaUploadFrame& operator=(const CudaUploadFrame& other) = delete;

  TaskExecStatus Run();
  ~CudaUploadFrame() = default;
  CudaUploadFrame(CUstream stream);

private:
  /* First input is src Buffer.
   * Second input is dst Surface.
   */
  static const uint32_t numInputs = 2U;
  static const uint32_t numOutputs = 0U;
  CUstream m_stream;
};

class TC_CORE_EXPORT CudaDownloadSurface final : public Task {
public:
  CudaDownloadSurface() = delete;
  CudaDownloadSurface(const CudaDownloadSurface& other) = delete;
  CudaDownloadSurface& operator=(const CudaDownloadSurface& other) = delete;

  TaskExecStatus Run();
  ~CudaDownloadSurface() = default;
  CudaDownloadSurface(CUstream cuStream);

private:
  /* First input is src Surface.
   * Second input is dst Buffer.
   */
  static const uint32_t numInputs = 2U;
  static const uint32_t numOutputs = 0U;
  CUstream m_stream;
};

class TC_CORE_EXPORT DemuxFrame final : public Task
{
public:
  DemuxFrame() = delete;
  DemuxFrame(const DemuxFrame& other) = delete;
  DemuxFrame& operator=(const DemuxFrame& other) = delete;

  void GetParams(struct MuxingParams& params) const;
  int64_t TsFromTime(double ts_sec);
  int64_t TsFromFrameNumber(int64_t frame_num);
  void Flush();
  TaskExecStatus Run() final;
  ~DemuxFrame() final;
  static DemuxFrame* Make(const char* url, const char** ffmpeg_options,
                          uint32_t opts_size);

private:
  DemuxFrame(std::istream& i_str, const char** ffmpeg_options,
             uint32_t opts_size);
  DemuxFrame(const char* url, const char** ffmpeg_options, uint32_t opts_size);
  static const uint32_t numInputs = 2U;
  static const uint32_t numOutputs = 5U;
  struct DemuxFrame_Impl* pImpl = nullptr;
};

class TC_CORE_EXPORT ConvertSurface final : public Task {
public:
  ConvertSurface() = delete;
  ConvertSurface(const ConvertSurface& other) = delete;
  ConvertSurface& operator=(const ConvertSurface& other) = delete;

  ~ConvertSurface();
  ConvertSurface(Pixel_Format src, Pixel_Format dst, CUcontext ctx,
                 CUstream str);

  TaskExecStatus Run() final;

private:
  /* 0) Source Surface.
   * 1) Destination Surface.
   * 2) Colorspace conversion context.
   */
  static const uint32_t numInputs = 3U;
  /* 0) Task exec details.
   */
  static const uint32_t numOutputs = 1U;
  struct NppConvertSurface_Impl* pImpl;
};

class TC_CORE_EXPORT ConvertFrame final : public Task {
public:
  ConvertFrame() = delete;
  ConvertFrame(const ConvertFrame& other) = delete;
  ConvertSurface& operator=(const ConvertFrame& other) = delete;

  static ConvertFrame* Make(uint32_t width, uint32_t height,
                            Pixel_Format inFormat, Pixel_Format outFormat);

  ~ConvertFrame();

  TaskExecStatus Run() final;

private:
  static const uint32_t numInputs = 3U;
  static const uint32_t numOutputs = 2U;

  struct ConvertFrame_Impl* pImpl;

  ConvertFrame(uint32_t width, uint32_t height, Pixel_Format inFormat,
               Pixel_Format outFormat);
};

class TC_CORE_EXPORT ResizeSurface final : public Task {
public:
  ResizeSurface() = delete;
  ResizeSurface(const ResizeSurface& other) = delete;
  ResizeSurface& operator=(const ResizeSurface& other) = delete;

  ~ResizeSurface();
  ResizeSurface(Pixel_Format format, CUcontext ctx, CUstream str);

  TaskExecStatus Run() final;

private:
  /* 0) Source Surface.
   * 1) Destinaion Surface.
   */
  static const uint32_t numInputs = 2U;
  static const uint32_t numOutputs = 0U;

  struct ResizeSurface_Impl* pImpl;
};

class TC_CORE_EXPORT RemapSurface final : public Task
{
public:
  RemapSurface() = delete;
  RemapSurface(const RemapSurface& other) = delete;
  RemapSurface& operator=(const RemapSurface& other) = delete;

  static RemapSurface* Make(const float* x_map, const float* y_map,
                            uint32_t remap_w, uint32_t remap_h,
                            Pixel_Format format, CUcontext ctx, CUstream str);

  ~RemapSurface();

  TaskExecStatus Run() final;

private:
  static const uint32_t numInputs = 1U;
  static const uint32_t numOutputs = 1U;

  struct RemapSurface_Impl* pImpl;
  RemapSurface(const float* x_map, const float* y_map, uint32_t remap_w,
               uint32_t remap_h, Pixel_Format format, CUcontext ctx,
               CUstream str);
};
} // namespace VPF
