/*
 *  Copyright (c) 2024 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "rtc_resampler_impl.h"

#include "api/audio/audio_frame.h"
#include "audio/remix_resample.h"
#include "base/refcountedobject.h"
#include "rtc_base/checks.h"

namespace libwebrtc {

scoped_refptr<RTCResampler> RTCResampler::Create() {
  return scoped_refptr<RTCResampler>(new RefCountedObject<RTCResamplerImpl>());
}

RTCResamplerImpl::RTCResamplerImpl()
    : resampler_(std::make_unique<webrtc::PushResampler<int16_t>>()) {}

RTCResamplerImpl::~RTCResamplerImpl() = default;

int RTCResamplerImpl::InitializeIfNeeded(int src_sample_rate_hz,
                                         int dst_sample_rate_hz,
                                         size_t num_channels) {
  return resampler_->InitializeIfNeeded(src_sample_rate_hz, dst_sample_rate_hz,
                                        num_channels);
}

int RTCResamplerImpl::Resample(const int16_t* src,
                               size_t src_length,
                               int16_t* dst,
                               size_t dst_capacity) {
  return resampler_->Resample(src, src_length, dst, dst_capacity);
}

int RTCResamplerImpl::RemixAndResample(const int16_t* src_data,
                                       size_t src_samples_per_channel,
                                       size_t src_num_channels,
                                       int src_sample_rate_hz,
                                       uint32_t src_timestamp,
                                       int dst_sample_rate_hz,
                                       size_t dst_num_channels,
                                       int16_t* dst_data,
                                       size_t dst_capacity,
                                       size_t* dst_samples_per_channel,
                                       uint32_t* dst_timestamp) {
  // 验证输入参数有效性
  if (!src_data || !dst_data || !dst_samples_per_channel || !dst_timestamp) {
    return -1;  // 无效参数
  }

  if (src_samples_per_channel == 0 || src_num_channels == 0 ||
      src_sample_rate_hz <= 0 || dst_sample_rate_hz <= 0 ||
      dst_num_channels == 0 || dst_capacity == 0) {
    return -1;  // 无效参数
  }

  // 计算需要的输出缓冲区大小
  const size_t max_output_samples = src_samples_per_channel *
                                    dst_sample_rate_hz / src_sample_rate_hz *
                                    dst_num_channels;
  if (dst_capacity < max_output_samples) {
    return -1;  // 输出缓冲区不足
  }

  // 创建输出AudioFrame
  webrtc::AudioFrame dst_frame;
  dst_frame.sample_rate_hz_ = dst_sample_rate_hz;
  dst_frame.num_channels_ = dst_num_channels;
  dst_frame.channel_layout_ = dst_num_channels == 1
                                  ? webrtc::CHANNEL_LAYOUT_MONO
                                  : webrtc::CHANNEL_LAYOUT_STEREO;
  dst_frame.timestamp_ = src_timestamp;

  // 调用WebRTC的RemixAndResample函数（直接使用原始数据版本）
  webrtc::voe::RemixAndResample(src_data, src_samples_per_channel,
                                src_num_channels, src_sample_rate_hz,
                                resampler_.get(), &dst_frame);

  // 提取结果到输出参数
  const size_t output_samples =
      dst_frame.samples_per_channel_ * dst_frame.num_channels_;
  if (output_samples > dst_capacity) {
    return -1;  // 输出缓冲区不足
  }

  // 复制音频数据到输出缓冲区
  std::memcpy(dst_data, dst_frame.data(), output_samples * sizeof(int16_t));

  // 设置输出参数
  *dst_samples_per_channel = dst_frame.samples_per_channel_;
  *dst_timestamp = dst_frame.timestamp_;

  return 0;  // 成功
}

}  // namespace libwebrtc