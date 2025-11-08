#pragma once

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <opencv2/opencv.hpp>


class CameraWidget : public QWidget {
Q_OBJECT
public:
enum class Source { CSI, USB };


explicit CameraWidget(Source source, int w = 1280, int h = 720, int fps = 30, int flip = 0, QWidget* parent = nullptr);
~CameraWidget();


private slots:
void grabFrame();


private:
bool openCSI(int w, int h, int fps, int flip);
bool openUSB(int index, int w, int h, int fps);
static std::string gstreamerPipeline(int capture_width, int capture_height,
int display_width, int display_height,
int framerate, int flip_method);


QLabel* view_ {nullptr};
QTimer* timer_ {nullptr};
cv::VideoCapture cap_;
int outW_ {0}, outH_ {0};
};