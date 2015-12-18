#include <QPainter>

#include "disp.h"
#include "ui_disp.h"


Disp::Disp(Vrok::VUMeter *meter, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Disp),
    _meter(meter)
{
    timer.start(1);
    connect(&timer, SIGNAL(timeout()),this, SLOT(draw()));
    ui->setupUi(this);
}

void Disp::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QPen pen;
    pen.setWidth(10);

    painter.setPen(pen);
    painter.drawLine(0,20,_meter->GetValue(0)*100,20);
    painter.drawLine(0,40,_meter->GetValue(1)*100,40);
    painter.end();
}

Disp::~Disp()
{
    delete ui;
}

void Disp::draw()
{
    repaint();
}
