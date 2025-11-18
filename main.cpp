#include <QApplication>

#include "wordmemorizer.hpp"

static constexpr char* LOGO_PATH = ":/icons/logo.png";

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    WordMemorizer w;
    w.setMinimumSize(1100, 700);
    w.setWindowTitle("单词记忆助手");
    w.setWindowIcon(QIcon(LOGO_PATH));
    w.show();
    return a.exec();
}
