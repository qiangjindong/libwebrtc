#include "rtc_window_recorder_impl.h"

#include "api/sequence_checker.h"
#include "rtc_base/checks.h"
#include "third_party/libyuv/include/libyuv.h"
#ifdef WEBRTC_WIN
#include "modules/desktop_capture/win/window_capture_utils.h"
#endif
#include "rtc_base/logging.h"
#include "rtc_base/time_utils.h"

namespace libwebrtc {

RTCWindowRecorderImpl::RTCWindowRecorderImpl(
    RTCWindowRecorder::SourceId source_id,
    const webrtc::DesktopCaptureOptions& options)
    : source_id_(source_id), thread_(rtc::Thread::Create()) {
  RTC_DCHECK(thread_);
  thread_->Start();
  thread_->BlockingCall([this, options] {
    capturer_ =
        webrtc::DesktopCapturer::CreateWindowCapturer(options, &is_use_wgc_);
  });
}

RTCWindowRecorderImpl::~RTCWindowRecorderImpl() {
  thread_->Stop();
  capturer_.reset();
}

void RTCWindowRecorderImpl::RegisterObserver(
    RTCWindowRecorderOberver* observer) {
  if (thread_->IsCurrent()) {
    observer_ = observer;
  } else {
    thread_->BlockingCall([this, observer] { observer_ = observer; });
  }
}

void RTCWindowRecorderImpl::UnregisterObserver() {
  if (thread_->IsCurrent()) {
    observer_ = nullptr;
  } else {
    thread_->BlockingCall([this] { observer_ = nullptr; });
  }
}

RTCWindowRecorder::State RTCWindowRecorderImpl::Start(uint32_t fps) {
  using State = RTCWindowRecorder::State;

  if (state_ == State::kRunning) {
    return state_;
  }

  if (fps == 0) {
    state_ = State::kFailed;
    return state_;
  }

  if (fps >= 60) {
    capture_delay_ = uint32_t(1000.0 / 60.0);
  } else {
    capture_delay_ = uint32_t(1000.0 / fps);
  }

  auto source_id = static_cast<webrtc::DesktopCapturer::SourceId>(source_id_);
  if (source_id != -1) {
    if (!capturer_->SelectSource(source_id)) {
      state_ = State::kFailed;
      return state_;
    }
    if (!capturer_->FocusOnSelectedSource()) {
      state_ = State::kFailed;
      return state_;
    }
  }

  thread_->BlockingCall([this] { capturer_->Start(this); });
  state_ = State::kRunning;
  thread_->PostTask([this] { CaptureFrame(); });
  return state_;
}

void RTCWindowRecorderImpl::Stop() {
  state_ = RTCWindowRecorder::State::kStopped;
}

bool RTCWindowRecorderImpl::IsRecording() {
  return state_ == RTCWindowRecorder::State::kRunning;
}

static int filterException(int code, PEXCEPTION_POINTERS ex) {
  return EXCEPTION_EXECUTE_HANDLER;
}

void RTCWindowRecorderImpl::OnCaptureResult(
    webrtc::DesktopCapturer::Result result,
    std::unique_ptr<webrtc::DesktopFrame> frame) {
  if (result == webrtc::DesktopCapturer::Result::ERROR_TEMPORARY) {
    return;
  }

  int width = frame->size().width();
  int height = frame->size().height();

#ifdef WEBRTC_WIN
  webrtc::DesktopRect rect_ = webrtc::DesktopRect::MakeWH(width, height);
  if (!is_use_wgc_) {
    webrtc::GetWindowRect(reinterpret_cast<HWND>(source_id_), &rect_);
  }

  __try
#endif
  {
    if (!i420_buffer_ || !i420_buffer_.get() ||
        i420_buffer_->width() * i420_buffer_->height() != width * height) {
      i420_buffer_ = webrtc::I420Buffer::Create(width, height);
    }

    libyuv::ConvertToI420(frame->data(), 0, i420_buffer_->MutableDataY(),
                          i420_buffer_->StrideY(), i420_buffer_->MutableDataU(),
                          i420_buffer_->StrideU(), i420_buffer_->MutableDataV(),
                          i420_buffer_->StrideV(), 0, 0,
#ifdef WEBRTC_WIN
                          rect_.width(), rect_.height(),
#else
                          width, height,
#endif
                          width, height, libyuv::kRotate0, libyuv::FOURCC_ARGB);

    if (observer_) {
      scoped_refptr<RTCWindowRecorder::I420Buffer> buffer(
          new RefCountedObject<RTCWindowRecorderI420BufferImpl>(i420_buffer_));
      observer_->OnFrameCaptured(std::move(buffer), rtc::TimeMillis());
    }
  }
#ifdef WEBRTC_WIN
  __except (filterException(GetExceptionCode(), GetExceptionInformation())) {
  }
#endif
}

void RTCWindowRecorderImpl::CaptureFrame() {
  RTC_DCHECK_RUN_ON(thread_.get());
  if (state_ == RTCWindowRecorder::State::kRunning) {
    capturer_->CaptureFrame();
    thread_->PostDelayedHighPrecisionTask(
        [this]() { CaptureFrame(); },
        webrtc::TimeDelta::Millis(capture_delay_));
  }
}

}  // namespace libwebrtc
