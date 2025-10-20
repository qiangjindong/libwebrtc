#ifndef LIB_WEBRTC_AUDIO_TRACK_IMPL_HXX
#define LIB_WEBRTC_AUDIO_TRACK_IMPL_HXX

#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "common_audio/resampler/include/push_resampler.h"
#include "common_audio/vad/include/webrtc_vad.h"
#include "media/engine/webrtc_video_engine.h"
#include "media/engine/webrtc_voice_engine.h"
#include "pc/media_session.h"
#include "rtc_audio_track.h"
#include "rtc_base/logging.h"
#include "rtc_base/synchronization/mutex.h"

namespace libwebrtc {

class AudioTrackSinkAdapter : public webrtc::AudioTrackSinkInterface {
 public:
  AudioTrackSinkAdapter(RTCAudioTrackSinkInterface* sink,
                        const string& track_id)
      : sink_(sink), track_id_(track_id) {}

  virtual ~AudioTrackSinkAdapter() {
    RTC_LOG(LS_INFO) << __FUNCTION__ << ": dtor";
  }

  void OnData(const void* audio_data,
              int bits_per_sample,
              int sample_rate,
              size_t number_of_channels,
              size_t number_of_frames,
              absl::optional<int64_t> absolute_capture_timestamp_ms) override {
    // RTC_LOG(LS_INFO) << __FUNCTION__
    //                  << ": number_of_channels=" << number_of_channels
    //                  << ", track_id_=" << track_id_.c_string();
    if (sink_) {
      // absolute_capture_timestamp_ms 获取值
      int64_t timestamp = absolute_capture_timestamp_ms.value_or(-1);
      sink_->OnData(audio_data, bits_per_sample, sample_rate,
                    number_of_channels, number_of_frames, timestamp, track_id_);
    }
  }

  void OnData(const void* audio_data,
              int bits_per_sample,
              int sample_rate,
              size_t number_of_channels,
              size_t number_of_frames) override {
    OnData(audio_data, bits_per_sample, sample_rate, number_of_channels,
           number_of_frames,
           /*absolute_capture_timestamp_ms=*/absl::nullopt);
  }

 private:
  RTCAudioTrackSinkInterface* sink_;
  string track_id_;
};

class AudioTrackImpl : public RTCAudioTrack {
 public:
  AudioTrackImpl(rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track);

  virtual ~AudioTrackImpl();

  virtual void SetVolume(double volume) override;

  virtual const string kind() const override { return kind_; }

  virtual const string id() const override { return id_; }

  virtual bool enabled() const override { return rtc_track_->enabled(); }

  virtual bool set_enabled(bool enable) override {
    return rtc_track_->set_enabled(enable);
  }

  rtc::scoped_refptr<webrtc::AudioTrackInterface> rtc_track() {
    return rtc_track_;
  }

  virtual RTCTrackState state() const override {
    return static_cast<RTCTrackState>(rtc_track_->state());
  }

  virtual void AddSink(RTCAudioTrackSinkInterface* sink) override;
  virtual void RemoveSink(RTCAudioTrackSinkInterface* sink) override;

 private:
  rtc::scoped_refptr<webrtc::AudioTrackInterface> rtc_track_;
  string id_, kind_;

  webrtc::Mutex mutex_;
  std::unordered_map<RTCAudioTrackSinkInterface*,
                     std::unique_ptr<AudioTrackSinkAdapter>>
      sinks_ RTC_GUARDED_BY(mutex_);
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_AUDIO_TRACK_IMPL_HXX
