/******************************************************/
/*                                                    */
/* tinwindow.cpp - window for viewing TIN             */
/*                                                    */
/******************************************************/
/* Copyright 2017 Pierre Abbat.
 * This file is part of Bezitopo.
 * 
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bezitopo. If not, see <http://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <cmath>
#include "tinwindow.h"
#include "zoom.h"
#include "test.h"

using namespace std;

TinCanvas::TinCanvas(QWidget *parent):QWidget(parent)
{
  setAutoFillBackground(true);
  setBackgroundRole(QPalette::Base);
  setPen(QPen(Qt::black));
  doc.pl.resize(2);
  aster(doc,100);
  doc.pl[1].maketin("",false);
  doc.pl[1].makegrad(0.);
  doc.pl[1].maketriangles();
  doc.pl[1].setgradient();
  doc.pl[1].makeqindex();
  doc.pl[1].findcriticalpts();
  doc.pl[1].addperimeter();
  sizeToFit();
  show();
}

QPointF TinCanvas::worldToWindow(xy pnt)
{
  pnt.roscat(worldCenter,rotation,zoomratio(scale)*windowSize,windowCenter);
  QPointF ret(pnt.getx(),height()-pnt.gety());
  return ret;
}

xy TinCanvas::windowToWorld(QPointF pnt)
{
  xy ret(pnt.x(),height()-pnt.y());
  ret.roscat(windowCenter,-rotation,zoomratio(-scale)/windowSize,worldCenter);
  return ret;
}

void TinCanvas::setPen(const QPen &qpen)
{
  pen=qpen;
}

void TinCanvas::setBrush(const QBrush &qbrush)
{
  brush=qbrush;
}

void TinCanvas::sizeToFit()
{
  double top,left,bottom,right;
  int plnum;
  int vscale,hscale;
  plnum=doc.pl.size()-1;
  if (plnum<0)
  {
    top=1;
    left=-1;
    bottom=-1;
    right=1;
  }
  else
  {
    top=-doc.pl[plnum].dirbound(DEG270-rotation);
    left=doc.pl[plnum].dirbound(-rotation);
    bottom=doc.pl[plnum].dirbound(DEG90-rotation);
    right=-doc.pl[plnum].dirbound(DEG180-rotation);
    if (top<=bottom)
    {
      top=1;
      bottom=-1;
    }
    if (left>=right)
    {
      left=-1;
      right=1;
    }
  }
  worldCenter=xy((left+right)/2,(top+bottom)/2);
  if (windowSize)
  {
    vscale=largestFit(height()/windowSize/(top-bottom));
    hscale=largestFit(width()/windowSize/(right-left));
  }
  else
  { // If the widget size is not known yet, assume it's square.
    vscale=largestFit(M_SQRT2/(top-bottom));
    hscale=largestFit(M_SQRT2/(right-left));
  }
  scale=(hscale<vscale)?hscale:vscale;
}

void TinCanvas::paintEvent(QPaintEvent *event)
{
  int i,plnum;
  QPainter painter(this);
  segment seg;
  painter.setPen(pen);
  painter.setBrush(brush);
  painter.setRenderHint(QPainter::Antialiasing,true);
  plnum=doc.pl.size()-1;
  for (i=0;plnum>=0 && i<doc.pl[plnum].edges.size();i++)
  {
    seg=doc.pl[plnum].edges[i].getsegment();
    painter.drawLine(worldToWindow(seg.getstart()),worldToWindow(seg.getend()));
  }
}

void TinCanvas::setSize()
{
  windowCenter=xy(width(),height())/2.;
  windowSize=1/sqrt(1/sqr(width())+1/sqr(height()));
}

void TinCanvas::resizeEvent(QResizeEvent *event)
{
  setSize();
  QWidget::resizeEvent(event);
}

TinWindow::TinWindow(QWidget *parent):QMainWindow(parent)
{
  resize(707,500);
  setWindowTitle(QApplication::translate("main", "ViewTIN"));
  show();
  toolbar=new QToolBar(this);
  addToolBar(Qt::TopToolBarArea,toolbar);
  canvas=new TinCanvas(this);
  setCentralWidget(canvas);
  canvas->show();
  makeActions();
}

TinWindow::~TinWindow()
{
  unmakeActions();
  delete canvas;
}

void TinWindow::makeActions()
{
  int i;
  zoomButtons[0]=new ZoomButton(this,-10);
  zoomButtons[0]->setIcon(QIcon(":/tenth.png"));
  zoomButtons[1]=new ZoomButton(this,-3);
  zoomButtons[1]->setIcon(QIcon(":/half.png"));
  zoomButtons[2]=new ZoomButton(this,-1);
  zoomButtons[2]->setIcon(QIcon(":/four-fifths.png"));
  zoomButtons[3]=new ZoomButton(this,1);
  zoomButtons[3]->setIcon(QIcon(":/five-fourths.png"));
  zoomButtons[4]=new ZoomButton(this,3);
  zoomButtons[4]->setIcon(QIcon(":/two.png"));
  zoomButtons[5]=new ZoomButton(this,10);
  zoomButtons[5]->setIcon(QIcon(":/ten.png"));
  for (i=0;i<6;i++)
    toolbar->addAction(zoomButtons[i]);
}

void TinWindow::unmakeActions()
{
  int i;
  for (i=0;i<6;i++)
  {
    toolbar->removeAction(zoomButtons[i]);
    delete zoomButtons[i];
    zoomButtons[i]=nullptr;
  }
}
