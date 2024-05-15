#ifndef DISP_H
#define DISP_H

#include "vumeter.h"
#include <QTimer>
#include <QWidget>

namespace Ui {
class Disp;
}

class Disp : public QWidget {
    Q_OBJECT

public:
    explicit Disp(Vrok::VUMeter *meter, QWidget *parent = 0);
    void paintEvent(QPaintEvent *e);

    ~Disp();
public slots:
    void draw();

private:
    Ui::Disp *ui;
    vrok::VUMeter *_meter;
    QTimer timer;
};

#endif // DISP_H
