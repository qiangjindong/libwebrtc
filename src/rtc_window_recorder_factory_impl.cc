#include "rtc_window_recorder_factory_impl.h"
#include "rtc_base/logging.h"
#include "rtc_window_recorder_impl.h"

namespace libwebrtc {

// static
scoped_refptr<RTCWindowRecorderFactory> RTCWindowRecorderFactory::Create() {
  return scoped_refptr<RTCWindowRecorderFactoryImpl>(
      new RefCountedObject<RTCWindowRecorderFactoryImpl>());
}

RTCWindowRecorderFactoryImpl::RTCWindowRecorderFactoryImpl() {
  options_ = webrtc::DesktopCaptureOptions::CreateDefault();
  options_.set_detect_updated_region(true);
#ifdef WEBRTC_WIN
  options_.set_allow_directx_capturer(true);
  options_.set_allow_wgc_screen_capturer(true);
  options_.set_allow_wgc_window_capturer(true);
  options_.set_allow_wgc_capturer_fallback(true);
  options_.set_allow_wgc_zero_hertz(true);
#endif
}

RTCWindowRecorderFactoryImpl::~RTCWindowRecorderFactoryImpl() {}

scoped_refptr<RTCWindowRecorder>
RTCWindowRecorderFactoryImpl::CreateWindowRecorder(
    RTCWindowRecorder::SourceId source_id) {
#if defined(WEBRTC_WIN)
  return scoped_refptr<RTCWindowRecorderImpl>(
      new RefCountedObject<RTCWindowRecorderImpl>(source_id, options_));
#else
  RTC_LOG(LS_WARNING)
      << "RTCWindowRecorder is only supported on Windows, skip creating.";
  (void)source_id;
  return nullptr;
#endif
}

}  // namespace libwebrtc
