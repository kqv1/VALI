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

#include "CudaUtils.hpp"
#include "PyNvCodec.hpp"
#include "memory"

using namespace VPF;
namespace py = pybind11;

constexpr auto TASK_EXEC_SUCCESS = TaskExecStatus::TASK_EXEC_SUCCESS;
constexpr auto TASK_EXEC_FAIL = TaskExecStatus::TASK_EXEC_FAIL;

PySurfaceDownloader::PySurfaceDownloader(uint32_t gpu_id) {
  upDownloader = std::make_unique<CudaDownloadSurface>(
      CudaResMgr::Instance().GetStream(gpu_id));
}

PySurfaceDownloader::PySurfaceDownloader(CUstream str) {
  upDownloader = std::make_unique<CudaDownloadSurface>(str);
}

bool PySurfaceDownloader::Run(Surface& src, py::array& dst) {
  auto buffer =
      std::shared_ptr<Buffer>(Buffer::Make(dst.nbytes(), dst.mutable_data()));

  upDownloader->SetInput(&src, 0U);
  upDownloader->SetInput(buffer.get(), 1U);

  if (TASK_EXEC_FAIL == upDownloader->Execute()) {
    return false;
  }

  return true;
}

void Init_PySurfaceDownloader(py::module& m) {
  py::class_<PySurfaceDownloader>(m, "PySurfaceDownloader",
                                  "This class is used to copy Surface to numpy "
                                  "ndarray using CUDA DtoH memcpy.")
      .def(py::init<uint32_t>(), py::arg("gpu_id"),
           R"pbdoc(
        Constructor method.

        :param gpu_id: what GPU does Surface belong to
    )pbdoc")
      .def(py::init<uint32_t>(), py::arg("stream"),
           R"pbdoc(
        Constructor method.

        :param stream: CUDA stream to use for HtoD memcopy
    )pbdoc")
      .def("Run", &PySurfaceDownloader::Run, py::arg("src"), py::arg("dst"),
           py::call_guard<py::gil_scoped_release>(),
           R"pbdoc(
        Perform DtoH memcpy.

        :param src: input Surface
        :type src: Surface
        :param dst: output numpy array
        :type dst: numpy.ndarray
        :return: True in case of success False otherwise
        :rtype: Bool
    )pbdoc");
}