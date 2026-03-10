#ifndef LIB_WEBRTC_RTC_WINDOW_RECORDER_IMPL_HXX
#define LIB_WEBRTC_RTC_WINDOW_RECORDER_IMPL_HXX

#include "api/video/i420_buffer.h"
#include "modules/desktop_capture/desktop_capture_options.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "rtc_base/thread.h"
#include "rtc_window_recorder.h"

namespace libwebrtc {

class RTCWindowRecorderI420BufferImpl : public RTCWindowRecorder::I420Buffer {
 public:
  explicit RTCWindowRecorderI420BufferImpl(
      rtc::scoped_refptr<webrtc::I420Buffer> i420_buffer)
      : i420_buffer_(std::move(i420_buffer)) {}

  int width() const override { return i420_buffer_->width(); }

  int height() const override { return i420_buffer_->height(); }

  const uint8_t* DataY() const override { return i420_buffer_->DataY(); }

  const uint8_t* DataU() const override { return i420_buffer_->DataU(); }

  const uint8_t* DataV() const override { return i420_buffer_->DataV(); }

  int StrideY() const override { return i420_buffer_->StrideY(); }

  int StrideU() const override { return i420_buffer_->StrideU(); }

  int StrideV() const override { return i420_buffer_->StrideV(); }

 private:
  rtc::scoped_refptr<webrtc::I420Buffer> i420_buffer_;
};

class RTCWindowRecorderImpl : public RTCWindowRecorder,
                              public webrtc::DesktopCapturer::Callback {
 public:
  RTCWindowRecorderImpl(RTCWindowRecorder::SourceId source_id,
                        const webrtc::DesktopCaptureOptions& options);

  ~RTCWindowRecorderImpl();

  void RegisterObserver(RTCWindowRecorderOberver* observer) override;

  void UnregisterObserver() override;

  State Start(uint32_t fps) override;

  void Stop() override;

  bool IsRecording() override;

 protected:
  virtual void OnCaptureResult(
      webrtc::DesktopCapturer::Result result,
      std::unique_ptr<webrtc::DesktopFrame> frame) override;

 private:
  void CaptureFrame();

  RTCWindowRecorder::State state_{RTCWindowRecorder::State::kStopped};
#if defined(WEBRTC_WIN)
  RTCWindowRecorder::SourceId source_id_;
#endif
  RTCWindowRecorderOberver* observer_{nullptr};
  std::unique_ptr<rtc::Thread> thread_;
  std::unique_ptr<webrtc::DesktopCapturer> capturer_;
#if defined(WEBRTC_WIN)
  bool is_use_wgc_{false};
  uint32_t capture_delay_ = 1000;  // 1s
#endif
  rtc::scoped_refptr<webrtc::I420Buffer> i420_buffer_;
};

}  // namespace libwebrtc

#endif
