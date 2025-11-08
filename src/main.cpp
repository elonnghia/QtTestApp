#include <QApplication>
#include "CameraWidget.h"
#include <iostream>


int main(int argc, char *argv[]) {
QApplication app(argc, argv);


// Mặc định dùng CSI; truyền "--usb" để dùng USB (/dev/video0)
CameraWidget::Source src = CameraWidget::Source::CSI;
for (int i = 1; i < argc; ++i) {
if (std::string(argv[i]) == "--usb") src = CameraWidget::Source::USB;
}


CameraWidget w(src, /*w*/1280, /*h*/720, /*fps*/30, /*flip*/0);
w.setWindowTitle("Jetson Nano Camera - Qt5 + OpenCV");
w.resize(960, 600);
w.show();


return app.exec();
}