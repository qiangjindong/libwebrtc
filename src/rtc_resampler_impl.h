/*
 *  Copyright (c) 2024 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef LIB_WEBRTC_RTC_RESAMPLER_IMPL_H_
#define LIB_WEBRTC_RTC_RESAMPLER_IMPL_H_

#include <memory>
#include <vector>

#include "common_audio/resampler/include/push_resampler.h"
#include "rtc_resampler.h"

namespace libwebrtc {

class RTCResamplerImpl : public RTCResampler {
 public:
  RTCResamplerImpl();
  ~RTCResamplerImpl() override;

  int InitializeIfNeeded(int src_sample_rate_hz,
                         int dst_sample_rate_hz,
                         size_t num_channels) override;

  int Resample(const int16_t* src,
               size_t src_length,
               int16_t* dst,
               size_t dst_capacity) override;

  int RemixAndResample(const int16_t* src_data,
                       size_t src_samples_per_channel,
                       size_t src_num_channels,
                       int src_sample_rate_hz,
                       uint32_t src_timestamp,
                       int dst_sample_rate_hz,
                       size_t dst_num_channels,
                       int16_t* dst_data,
                       size_t dst_capacity,
                       size_t* dst_samples_per_channel,
                       uint32_t* dst_timestamp) override;

 private:
  std::unique_ptr<webrtc::PushResampler<int16_t>> resampler_;
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_RTC_RESAMPLER_IMPL_H_