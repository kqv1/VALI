/*
 * Copyright 2019 NVIDIA Corporation
 * Copyright 2021 Kognia Sports Intelligence
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

#include "PyNvCodec.hpp"

using namespace VPF;

namespace py = pybind11;

constexpr auto TASK_EXEC_SUCCESS = TaskExecStatus::TASK_EXEC_SUCCESS;
constexpr auto TASK_EXEC_FAIL = TaskExecStatus::TASK_EXEC_FAIL;

PySurfaceConverter::PySurfaceConverter(uint32_t width, uint32_t height,
                                       Pixel_Format inFormat,
                                       Pixel_Format outFormat, uint32_t gpuID)
    : outputFormat(outFormat) {
  upConverter.reset(ConvertSurface::Make(
      width, height, inFormat, outFormat, CudaResMgr::Instance().GetCtx(gpuID),
      CudaResMgr::Instance().GetStream(gpuID)));
  upCtxBuffer.reset(Buffer::MakeOwnMem(sizeof(ColorspaceConversionContext)));
}

PySurfaceConverter::PySurfaceConverter(uint32_t width, uint32_t height,
                                       Pixel_Format inFormat,
                                       Pixel_Format outFormat, CUstream str)
    : outputFormat(outFormat) {
  upConverter.reset(ConvertSurface::Make(width, height, inFormat, outFormat,
                                         GetContextByStream(str), str));
  upCtxBuffer.reset(Buffer::MakeOwnMem(sizeof(ColorspaceConversionContext)));
}

bool PySurfaceConverter::Execute(
    std::shared_ptr<Surface> src, std::shared_ptr<Surface> dst,
    std::shared_ptr<ColorspaceConversionContext> context,
    TaskExecDetails& details) {
  if (!src || !dst || !context) {
    return false;
  }

  upCtxBuffer->CopyFrom(sizeof(ColorspaceConversionContext), context.get());

  upConverter->ClearInputs();
  upConverter->SetInput((Token*)src.get(), 0U);
  upConverter->SetInput((Token*)dst.get(), 1U);
  upConverter->SetInput((Token*)upCtxBuffer.get(), 2U);

  auto ret = upConverter->Execute();

  auto pDetails = (Buffer*)upConverter->GetOutput(1U);
  if (pDetails) {
    details = *(pDetails->GetDataAs<TaskExecDetails>());
  }

  return (TASK_EXEC_SUCCESS == ret);
}

Pixel_Format PySurfaceConverter::GetFormat() { return outputFormat; }

void Init_PySurfaceConverter(py::module& m) {
  py::class_<PySurfaceConverter, std::shared_ptr<PySurfaceConverter>>(
      m, "PySurfaceConverter",
      "CUDA-accelerated converter between different pixel formats.")
      .def(py::init<uint32_t, uint32_t, Pixel_Format, Pixel_Format, uint32_t>(),
           py::arg("width"), py::arg("height"), py::arg("src_format"),
           py::arg("dst_format"), py::arg("gpu_id"),
           R"pbdoc(
        Constructor method.

        :param width: target Surface width
        :param height: target Surface height
        :param src_format: input Surface pixel format
        :param dst_format: output Surface pixel format
        :param gpu_id: what GPU to run conversion on
    )pbdoc")
      .def(py::init<uint32_t, uint32_t, Pixel_Format, Pixel_Format, size_t>(),
           py::arg("width"), py::arg("height"), py::arg("src_format"),
           py::arg("dst_format"), py::arg("stream"),
           R"pbdoc(
        Constructor method.

        :param width: target Surface width
        :param height: target Surface height
        :param src_format: input Surface pixel format
        :param dst_format: output Surface pixel format
        :param stream: CUDA stream to use for conversion
    )pbdoc")
      .def("Format", &PySurfaceConverter::GetFormat, R"pbdoc(
        Get pixel format.
    )pbdoc")
      .def(
          "Execute",
          [](std::shared_ptr<PySurfaceConverter> self,
             std::shared_ptr<Surface> src, std::shared_ptr<Surface> dst,
             std::shared_ptr<ColorspaceConversionContext> cc_ctx) {
            TaskExecDetails details;
            return std::make_tuple(self->Execute(src, dst, cc_ctx, details),
                                   details.info);
          },
          py::arg("src"), py::arg("dst"), py::arg("cc_ctx"),
          py::call_guard<py::gil_scoped_release>(),
          R"pbdoc(
        Perform pixel format conversion.

        :param src: input Surface. Must be of same format class instance was created with.
        :param dst: output Surface. Must be of suitable format.
        :param cc_ctx: colorspace conversion context. Describes color space and color range used for conversion.
        :return: tuple containing:
          success (Bool) True in case of success, False otherwise.
          info (TaskExecInfo) task execution information.
        :rtype: tuple
    )pbdoc");
}