#include "CameraWidget.h"
#include <QImage>
#include <QPixmap>
#include <QSizePolicy>
#include <QDebug>


CameraWidget::CameraWidget(Source source, int w, int h, int fps, int flip, QWidget* parent)
: QWidget(parent), outW_(w), outH_(h) {
auto *layout = new QVBoxLayout(this);
view_ = new QLabel(this);
view_->setAlignment(Qt::AlignCenter);
view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
layout->addWidget(view_);
setLayout(layout);


bool ok = false;
if (source == Source::CSI) {
ok = openCSI(w, h, fps, flip);
} else {
ok = openUSB(0, w, h, fps); // /dev/video0
}


if (!ok) {
view_->setText("❌ Không mở được camera");
return;
}


timer_ = new QTimer(this);
connect(timer_, &QTimer::timeout, this, &CameraWidget::grabFrame);
timer_->start(1000 / std::max(1, fps));
}


CameraWidget::~CameraWidget() {
if (cap_.isOpened()) cap_.release();
}


void CameraWidget::grabFrame() {
cv::Mat frame;
if (!cap_.isOpened() || !cap_.read(frame) || frame.empty()) {
view_->setText("⏳ Chờ khung hình...");
return;
}


// OpenCV trả về BGR, Qt cần RGB
cv::Mat rgb;
if (frame.channels() == 3) {
cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
} else if (frame.channels() == 4) {
cv::cvtColor(frame, rgb, cv::COLOR_BGRA2RGB);
} else {
cv::cvtColor(frame, rgb, cv::COLOR_GRAY2RGB);
}


QImage img(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
view_->setPixmap(QPixmap::fromImage(img).scaled(view_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}


bool CameraWidget::openUSB(int index, int w, int h, int fps) {
// Dùng V4L2 mặc định (OpenCV trên Jetson có hỗ trợ)
if (!cap_.open(index)) return false;
cap_.set(cv::CAP_PROP_FRAME_WIDTH, w);
cap_.set(cv::CAP_PROP_FRAME_HEIGHT, h);
cap_.set(cv::CAP_PROP_FPS, fps);
return cap_.isOpened();
}


bool CameraWidget::openCSI(int w, int h, int fps, int flip) {
}