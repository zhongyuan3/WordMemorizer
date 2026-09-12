#include <QApplication>

#include "main_window.h"

static constexpr const char *LOGO_PATH = ":/resources/icons/logo.png";

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.setMinimumSize(1100, 700);
    w.setWindowTitle("单词记忆助手");
    w.setWindowIcon(QIcon(LOGO_PATH));
    w.show();
    return a.exec();
}
