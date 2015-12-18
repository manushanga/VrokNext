#ifndef DISP_H
#define DISP_H

#include <QWidget>
#include <QTimer>
#include "vumeter.h"

namespace Ui {
class Disp;
}

class Disp : public QWidget
{
    Q_OBJECT

public:
    explicit Disp(Vrok::VUMeter *meter, QWidget *parent = 0);
    void paintEvent(QPaintEvent *e);

    ~Disp();
public slots:
    void draw();

private:
    Ui::Disp *ui;
    Vrok::VUMeter *_meter;
    QTimer timer;
};

#endif // DISP_H
