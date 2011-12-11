#include <QtGui/QApplication>
#include "SubtitlesEditor.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName("WZSubtitlesEditor");
    application.setApplicationVersion("1.0");
    application.setOrganizationName("Warzone2100");
    application.setOrganizationDomain("wz2100.net");

    MainWindow window;
    window.show();

    return application.exec();
}
