/*
 * Copyright 2019 NVIDIA Corporation
 * Copyright 2023 Roman Arzumanyan
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
#include <chrono>
#include <map>
#include <sstream>
#include <string>
#include <utility>

extern "C" {
#include <libavutil/dict.h>
#include <libavutil/error.h>
}

#define X_TEXTIFY(a) TEXTIFY(a)
#define TEXTIFY(a) #a
#define MAKE_PAIR(a)  std::make_pair(a, TEXTIFY(a))

// AVFormatContext timeout handler;
class TimeoutHandler {
  std::chrono::milliseconds m_timeout;
  std::chrono::time_point<std::chrono::system_clock> m_last_time;

public:
  ~TimeoutHandler() = default;

  TimeoutHandler(uint32_t timeout_ms) : m_timeout(timeout_ms) {}

  TimeoutHandler() {
    constexpr uint32_t default_timeout_ms = 3000U;
    m_timeout = std::chrono::milliseconds(default_timeout_ms);
  }

  bool IsTimeout() const {
    auto delay = std::chrono::system_clock::now() - m_last_time;
    return std::chrono::duration_cast<std::chrono::milliseconds>(delay) >
           m_timeout;
  }

  static int Check(void *self) {
    return self && static_cast<TimeoutHandler *>(self)->IsTimeout();
  }

  void Reset() { m_last_time = std::chrono::system_clock::now(); }
};

static std::string AvErrorToString(int av_error_code) {
  const auto buf_size = 1024U;
  char *err_string = (char *)calloc(buf_size, sizeof(*err_string));
  if (!err_string) {
    return std::string();
  }

  if (0 != av_strerror(av_error_code, err_string, buf_size - 1)) {
    free(err_string);
    std::stringstream ss;
    ss << "Unknown error with code " << av_error_code;
    return ss.str();
  }

  std::string str(err_string);
  free(err_string);
  return str;
}

static AVDictionary *
GetAvOptions(const std::map<std::string, std::string> &ffmpeg_options) {
  AVDictionary *options = nullptr;
  for (auto &pair : ffmpeg_options) {
    // Handle 'timeout' option separately
    if (pair.first == "timeout") {
      continue;
    }

    auto err =
        av_dict_set(&options, pair.first.c_str(), pair.second.c_str(), 0);

    if (err < 0) {
      av_dict_free(&options);
      std::stringstream ss;
      ss << "Can't set up dictionary option: " << pair.first << " "
         << pair.second << ": " << AvErrorToString(err) << "\n";
      throw std::runtime_error(ss.str());
    }
  }

  return options;
}

static std::string GetFormatName(Pixel_Format fmt) {
  static const std::map<Pixel_Format, std::string> names({
      MAKE_PAIR(UNDEFINED),
      MAKE_PAIR(Y),
      MAKE_PAIR(RGB),
      MAKE_PAIR(NV12),
      MAKE_PAIR(YUV420),
      MAKE_PAIR(RGB_PLANAR),
      MAKE_PAIR(BGR),
      MAKE_PAIR(YUV444),
      MAKE_PAIR(RGB_32F),
      MAKE_PAIR(RGB_32F_PLANAR),
      MAKE_PAIR(YUV422),
      MAKE_PAIR(P10),
      MAKE_PAIR(P12),
      MAKE_PAIR(YUV444_10bit),
      MAKE_PAIR(YUV420_10bit),
      MAKE_PAIR(GRAY12),
  });

  auto it = names.find(fmt);
  if (it == names.end()) {
    return "";
  }

  return it->second;
}