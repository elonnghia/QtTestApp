#include "CameraWidget.h"
#include <QImage>
#include <QPixmap>
#include <QSizePolicy>
#include <QDir>
#include <QDateTime>
#include <QDebug>

CameraWidget::CameraWidget(Source source, int w, int h, int fps, int flip, QWidget* parent)
    : QWidget(parent), outW_(w), outH_(h) {
    auto *layout = new QVBoxLayout(this);
    view_ = new QLabel(this);
    view_->setAlignment(Qt::AlignCenter);
    view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(view_);

    captureBtn_ = new QPushButton("📸 Chụp hình", this);
    layout->addWidget(captureBtn_);

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
    connect(captureBtn_, &QPushButton::clicked, this, &CameraWidget::captureImage);
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

    // Lưu frame hiện tại
    currentFrame_ = frame.clone();

    // Xoay hình 180 độ
    cv::flip(frame, frame, -1);

    // Chuyển sang RGB để hiển thị
    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);

    QImage img(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
    view_->setPixmap(QPixmap::fromImage(img).scaled(view_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void CameraWidget::captureImage() {
    if (currentFrame_.empty()) {
        qDebug() << "⚠️ Không có khung hình để chụp.";
        return;
    }

    // Tạo thư mục lưu
    QString saveDir = QDir::homePath() + "/camera-picture/";
    QDir().mkpath(saveDir);

    // Xoay 180° trước khi lưu để đúng hướng
    cv::Mat flipped;
    cv::flip(currentFrame_, flipped, -1); // -1 = xoay 180° (lật ngang + dọc)

    QString filename = saveDir + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".jpg";
    cv::imwrite(filename.toStdString(), flipped);

    qDebug() << "💾 Đã lưu hình tại:" << filename;
}

bool CameraWidget::openUSB(int index, int w, int h, int fps) {
    if (!cap_.open(index)) return false;
    cap_.set(cv::CAP_PROP_FRAME_WIDTH, w);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, h);
    cap_.set(cv::CAP_PROP_FPS, fps);
    return cap_.isOpened();
}

bool CameraWidget::openCSI(int w, int h, int fps, int flip) {
    std::string pipeline = gstreamerPipeline(w, h, w, h, fps, flip);
    return cap_.open(pipeline, cv::CAP_GSTREAMER);
}

std::string CameraWidget::gstreamerPipeline(int capture_width, int capture_height,
                                            int display_width, int display_height,
                                            int framerate, int flip_method) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer),
             "nvarguscamerasrc ! video/x-raw(memory:NVMM), width=%d, height=%d, framerate=%d/1, format=NV12 ! "
             "nvvidconv flip-method=%d ! video/x-raw, width=%d, height=%d, format=BGRx ! "
             "videoconvert ! video/x-raw, format=BGR ! appsink drop=true max-buffers=1 sync=false",
             capture_width, capture_height, framerate,
             flip_method, display_width, display_height);
    return std::string(buffer);
}
