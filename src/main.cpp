#include <QApplication>
#include "CameraWidget.h"
#include <iostream>


int main(int argc, char *argv[]) {
QApplication app(argc, argv);


CameraWidget::Source src = CameraWidget::Source::CSI;
for (int i = 1; i < argc; ++i) {
if (std::string(argv[i]) == "--usb") src = CameraWidget::Source::USB;
}


CameraWidget w(src, 1280, 720, 30, 0);
w.setWindowTitle("Jetson Nano Camera - Qt5 + OpenCV");
w.resize(960, 600);
w.show();


return app.exec();
}