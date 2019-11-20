#include "dabiaoqian.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	dabiaoqian w;
	w.show();
	return a.exec();
}
