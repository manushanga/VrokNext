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
    double val[2],clip[2];
    _meter->GetValues(0, val[0], clip[0]);
    _meter->GetValues(1, val[1], clip[1]);

    Vrok::Clip(val[0],0.0,1.0);
    Vrok::Clip(val[1],0.0,1.0);

    painter.setPen(pen);
    painter.drawLine(0,20,val[0]*100,20);
    painter.drawLine(0,40,val[1]*100,40);

    QColor color1("red"),color2("red");
    color1.setAlpha(clip[0]*255);
    color2.setAlpha(clip[1]*255);

    pen.setColor(color1);
    painter.setPen(pen);
    painter.drawLine(100,20,110,20);
    pen.setColor(color2);
    painter.setPen(pen);
    painter.drawLine(100,40,110,40);
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
