#ifndef LIB_WEBRTC_RTC_WINDOW_RECORDER_FACTORY_IMPL_HXX
#define LIB_WEBRTC_RTC_WINDOW_RECORDER_FACTORY_IMPL_HXX

#include "modules/desktop_capture/desktop_capture_options.h"
#include "rtc_window_recorder.h"
#include "rtc_window_recorder_factory.h"

namespace libwebrtc {

class RTCWindowRecorderFactoryImpl : public RTCWindowRecorderFactory {
 public:
  RTCWindowRecorderFactoryImpl();

  ~RTCWindowRecorderFactoryImpl() override;

  scoped_refptr<RTCWindowRecorder> CreateWindowRecorder(
      RTCWindowRecorder::SourceId source_id) override;

 private:
  webrtc::DesktopCaptureOptions options_;
};

}  // namespace libwebrtc

#endif
