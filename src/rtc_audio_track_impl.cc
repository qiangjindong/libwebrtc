#include "rtc_audio_track_impl.h"

namespace libwebrtc {

AudioTrackImpl::AudioTrackImpl(
    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track)
    : rtc_track_(audio_track) {
  RTC_LOG(LS_INFO) << __FUNCTION__ << ": ctor ";
  id_ = rtc_track_->id();
  kind_ = rtc_track_->kind();
}

AudioTrackImpl::~AudioTrackImpl() {
  RTC_LOG(LS_INFO) << __FUNCTION__ << ": dtor " << sinks_.size();

  webrtc::MutexLock lock(&mutex_);
  for (auto const& [sink, adapter] : sinks_) {
    rtc_track_->RemoveSink(adapter.get());
  }
}

void AudioTrackImpl::SetVolume(double volume) {
  rtc_track_->GetSource()->SetVolume(volume);
}

void AudioTrackImpl::AddSink(RTCAudioTrackSinkInterface* sink) {
  webrtc::MutexLock lock(&mutex_);
  if (sinks_.find(sink) == sinks_.end()) {
    std::unique_ptr<AudioTrackSinkAdapter> adapter =
        std::make_unique<AudioTrackSinkAdapter>(sink, id_);
    rtc_track_->AddSink(adapter.get());
    sinks_.insert(std::pair<RTCAudioTrackSinkInterface*,
                            std::unique_ptr<AudioTrackSinkAdapter>>(
        sink, std::move(adapter)));
  }
}

void AudioTrackImpl::RemoveSink(RTCAudioTrackSinkInterface* sink) {
  webrtc::MutexLock lock(&mutex_);
  auto it = sinks_.find(sink);
  if (it != sinks_.end()) {
    rtc_track_->RemoveSink(it->second.get());
    RTC_LOG(LS_INFO) << __FUNCTION__ << ": adapter=" << it->second;
    sinks_.erase(it);
  }
}

}  // namespace libwebrtc
