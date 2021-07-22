#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QPainter>
#include <QSlider>
#include <math.h>

// ウイジェットのサイズをLayoutオブジェクト内で固定するには，sizePolicyをfixedにして，minimumとmaximumに同じ値を入れる
//https://digirakuda.org/blog/2018/03/14/post-41/

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setMouseTracking(true);

    ui->SliderZoom->setEnabled(false); // disable Zoom slider at initial
    index = 0;
/*
    imageFile = QImage("/Users/akita/Desktop/testL.jpg");
    fImageLoaded = true;
    ui->SliderZoom->setEnabled(true);
*/
}

// 描画時のpaintイベント
// http://memotiyou.blogspot.com/2011/12/qt-c-qpainterqimage_755.html
// QPainter
// https://kiwamiden.com/how-to-draw-and-save-to-qimage-with-qpainter
// 画像表示
// https://mf-atelier.sakura.ne.jp/mf-atelier2/qt_image/
// 半透明の画像
// https://base64.work/so/qt4/812458
// addItem()でのZ方向の重ね合わせ
//http://vivi.dyndns.org/vivi/docs/Qt/graphics.html

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event){
    QPainter painter(this);
    int imageWidth, imageHeight, drawingWidth, drawingHeight;

    // centralWidgetへ配置したオブジェクトのサイズを、MainWindowのサイズに合わせて自動調整
    // https://tips.crosslaboratory.com/post/qt_widget_size_does_not_change/
    drawingWidth = ui->horizontalLayout->geometry().width() - ui->verticalScrollBar->geometry().width();
    drawingHeight = ui->horizontalLayout->geometry().height();
    painter.setPen(QPen(Qt::black));
    painter.drawRect(0, 0, drawingWidth, drawingHeight);

    // Painterに画像をPixmapで描画
    //  https://living-sun.com/ja/qt/736351-qpainterdrawpixmap-doesn39t-look-good-and-has-low-quality-qt-qwidget-qpainter-qpixmap-qt55.html
//    painter.scale(0.1, 0.1); // painterを拡大縮小

    // Painterで画像を表示
    // https://mf-atelier.sakura.ne.jp/mf-atelier2/qt_image/?path_info=program/Qt/qt_image.html
    if (fImageLoaded == true){
//        QPixmap img;
//        img.convertFromImage(imageFile);
        QImage img = QImage(imageFile);
        imageWidth = img.width(); imageHeight = img.height();
        img = img.scaled(imageWidth * mag, imageHeight * mag, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        // painterの拡大縮小変換
//        QTransform matrix;
//        matrix.scale(1, 1);
//        painter.setWorldTransform(matrix);

        // viewportとwindowの関係
        // https://runebook.dev/ja/docs/qt/coordsys
        // QPainterの座標＝論理座標
        // setWindow: 論理座標の変換 setWindow(-50, -50, 100, 100)で，論理座標の左上が(-50,-50)になる

        painter.setClipRegion(QRegion(0, 0, drawingWidth, drawingHeight));
//        img.scroll(hScroll * imageWidth, vScroll * imageHeight, img.rect()); // scrollだと移動前のQPixmapが残る
//        painter.drawPixmap(0, 0, img); // img at (0, 0)
        posX = (int)((imageWidth * mag - drawingWidth) * hScroll);
        posY = (int)((imageHeight * mag - drawingHeight) * vScroll);
        painter.drawImage(0, 0, img, posX, posY);
        /*
         <--W-->     <--W*mag-->    W=imageWidth, Y=drawingWidth
         +-----+  -> +---------+
                     +-Y-+        hScroll=0.0
                           +-Y-+  hScroll=1.0
         */
//        qDebug("%d %d / %d %d / %.1f / %d %d / %d", drawingWidth, drawingHeight, imageWidth, imageHeight, mag, posX, posY, fDragging);
    }
    painter.setPen(Qt::red);
    painter.drawRect(rect_draw[0]);
    painter.setPen(Qt::blue);
    painter.drawRect(rect_draw[1]);
}


// *.hで宣言した独自のスロットはここで定義
void MainWindow::OpenFileDialog()
{
    // ファイル選択ダイアログ
    // https://www.hirominomi.com/fileselectbutton/
    QString strFileName = QFileDialog::getOpenFileName(this, tr("ファイル選択画面"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    //ui->label->setText(strFileName);
    // 画像ファイルの読み込みとシーンへのはりつけ
    //    QImage image(strFileName);
    imageFile = QImage(strFileName);
    fImageLoaded = true;
    ui->SliderZoom->setEnabled(true);

    // 画像ファイルの形式
    // https://qiita.com/kanryu/items/89812b362dfd581a4c10
}


// マウスのイベントの取得(Qt4とQt5でぜんぜん違う）
// http://dorafop.my.coocan.jp/Qt/Qt104.html
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int drawingHeight = ui->horizontalLayout->geometry().height();
        if (event->position().y() < drawingHeight){
            rect_draw[index].setTopLeft(event->position().toPoint());
        }
//        qDebug("MousePress x=%f,y=%f", event->position().x(), event->position().y());
/*
        if (index == 0){
            ui->plainTextEditRect1X->setEnabled(true);
            ui->plainTextEditRect1Y->setEnabled(true);
            ui->plainTextEditRect2X->setEnabled(false);
            ui->plainTextEditRect2Y->setEnabled(false);
        }
        else{
            ui->plainTextEditRect1X->setEnabled(false);
            ui->plainTextEditRect1Y->setEnabled(false);
            ui->plainTextEditRect2X->setEnabled(true);
            ui->plainTextEditRect2Y->setEnabled(true);
        }
*/
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int drawingHeight = ui->horizontalLayout->geometry().height();
        if (event->position().y() < drawingHeight){
            rect_draw[index].setBottomRight(event->position().toPoint());
        }
//        qDebug("MouseRelease x=%f,y=%f", event->position().x(), event->position().y());
    }
}

// イベントフィルターを使う方法もある
// https://www.it-swarm-ja.com/ja/qt/qt%E3%81%A7mousemoveevents%E3%82%92%E5%8F%96%E5%BE%97%E3%81%99%E3%82%8B/968683073/
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
//    qDebug("MouseMove x=%f,y=%f", event->position().x(), event->position().y());
    // QStringのフォーマットの指定
    // http://dorafop.my.coocan.jp/Qt/Qt199.html

    // ステータスバーに表示
    // http://memotiyou.blogspot.com/2012/03/qt-c_238.html
    ui->statusbar->showMessage(QString("(%1 %2)").arg(event->position().x()).arg(event->position().y()));
    rect_draw[index].setBottomRight(event->position().toPoint());
    int width = rect_draw[index].right() - rect_draw[index].left(); if (width < 0) width = -width;
    int height = rect_draw[index].bottom() - rect_draw[index].top(); if (height < 0) height = -height;
    if (index == 0){
        ui->radioButtonRect1->setText((QString("Rect1(%1 %2)").arg(width).arg(height)));
//        ui->plainTextEditRect1X->setPlainText(QString("%1").arg(width));
//        ui->plainTextEditRect1Y->setPlainText(QString("%1").arg(height));
    }
    else{
        ui->radioButtonRect2->setText((QString("Rect2(%1 %2)").arg(width).arg(height)));
//        ui->plainTextEditRect2X->setPlainText(QString("%1").arg(width));
//        ui->plainTextEditRect2Y->setPlainText(QString("%1").arg(height));
    }
    update();
}

// Qtでマウスホイール
// https://qt-is-cute.com/qwidget-use-wheel-event/
// https://doc.qt.io/qt-5/qwheelevent.html
void MainWindow::wheelEvent(QWheelEvent *event)
{
    int dx, dy;
    dx = event->angleDelta().x();
    dy = event->angleDelta().y();
#define TH 10
    int v;
    v = ui->horizontalScrollBar->value() - dx / TH;
    if (v < ui->horizontalScrollBar->minimum()) v = ui->horizontalScrollBar->minimum();
    if (v > ui->horizontalScrollBar->maximum()) v = ui->horizontalScrollBar->maximum();
    ui->horizontalScrollBar->setValue(v);
    v = ui->verticalScrollBar->value() - dy / TH;
    if (v < ui->verticalScrollBar->minimum()) v = ui->verticalScrollBar->minimum();
    if (v > ui->verticalScrollBar->maximum()) v = ui->verticalScrollBar->maximum();
    ui->verticalScrollBar->setValue(v);
}

// Scene/View/Itemとtransformの関係
// https://fereria.github.io/reincarnation_tech/11_PySide/02_Tips/GraphicsView/graphicsView_01/

// Sliderの使い方
// https://natural966.wordpress.com/2011/02/15/%E3%80%8Cqt-creator%E3%80%8D%E5%85%A5%E9%96%80%E8%80%85%E3%81%AE%E3%83%A1%E3%83%A2-2/
void MainWindow::onSliderZoomChanged(int value)
{
    // 0          50       99
    // mag/100    mag      mag*100
    // 10^(-2)    10^(0)   10^(2)
    //    double p = ((double)(value - 50) / 50.0) * 2.0;
    double p = ((double)(value - 50) / 50.0) * 1.0; // x0.1 - x10
    mag = pow(10.0, p);
    // QStringで小数（桁数指定）のフォーマット
    // https://sites.google.com/site/ubuntsdeqthajimemashita/qstringno-shu-shi-she-ding
    ui->labelZoom->setText(QString("x%1").arg(mag, 4, 'f', 2));
//    QTransform trans = QTransform();
//    trans.scale(m/mag, m/mag);
//    imageFile_item->setTransform(trans);
    update();
}

void MainWindow::onRadiobuttonClicked()
{
    if (ui->radioButtonRect1->isChecked() == true) index = 0; else index = 1;
}

void MainWindow::onHorizontalScrollBarChanged(int value)
{
    hScroll = (double)(value - ui->horizontalScrollBar->minimum())/ (ui->horizontalScrollBar->maximum() - ui->horizontalScrollBar->minimum());
    update();
}
void MainWindow::onVerticalScrollBarChanged(int value)
{
    vScroll = (double)(value - ui->verticalScrollBar->minimum()) / (ui->verticalScrollBar->maximum() - ui->verticalScrollBar->minimum());
    update();
}

void MainWindow::set_other_Rect(int index)
{
    double rect_ratio_x, rect_ratio_y;
    if (rect_draw[1 - index].width() != 0) rect_ratio_x = (double)rect_draw[index].width() / (double)rect_draw[1 - index].width(); else rect_ratio_x = 1;
    if (rect_draw[1 - index].height() != 0) rect_ratio_y = (double)rect_draw[index].height() / (double)rect_draw[1 - index].height(); else rect_ratio_y = 1;

    if (index == 0){
        ui->plainTextEditRect2X->setPlainText(QString("%1").arg(rect_physical[index].x() / rect_ratio_x, 5, 'f', 3));
        ui->plainTextEditRect2Y->setPlainText(QString("%1").arg(rect_physical[index].y() / rect_ratio_y, 5, 'f', 3));
    }
    else{
        ui->plainTextEditRect1X->setPlainText(QString("%1").arg(rect_physical[index].x() / rect_ratio_x, 5, 'f', 3));
        ui->plainTextEditRect1Y->setPlainText(QString("%1").arg(rect_physical[index].y() / rect_ratio_y, 5, 'f', 3));
    }
}

void MainWindow::normalize_rect_size()
{
    int i;
    for (i = 0; i < 2; i++){
        if (rect_draw[i].width() < 0) rect_draw[i].setWidth(-rect_draw[i].width());
        if (rect_draw[i].height() < 0) rect_draw[i].setHeight(-rect_draw[i].height());
    }
}

void MainWindow::onSetRect1X()
{
    rect_physical[0].setX(ui->plainTextEditRect1X->toPlainText().toDouble());
    if (rect_physical[0].x() != 0){
        double aspect;
        normalize_rect_size();
        qDebug("%d %d / %d %d", rect_draw[0].width(), rect_draw[0].height(), rect_draw[1].width(), rect_draw[1].height());
        if (rect_draw[0].height() != 0) aspect = (double)rect_draw[0].width() / (double)rect_draw[0].height(); else aspect = 1;

        if (aspect != 0) rect_physical[0].setY(rect_physical[0].x() / aspect); else rect_physical[0].setY(0);

        ui->plainTextEditRect1Y->setPlainText(QString("%1").arg(rect_physical[0].y(), 5, 'f', 3));
        set_other_Rect(0);
    }
}

void MainWindow::onSetRect1Y()
{
    rect_physical[0].setY(ui->plainTextEditRect1Y->toPlainText().toDouble());
    if (rect_physical[0].y() != 0){
        double aspect;
        normalize_rect_size();
        if (rect_draw[0].height() != 0) aspect = (double)rect_draw[0].width() / (double)rect_draw[0].height(); else aspect = 1;

        if (aspect != 0) rect_physical[0].setX(rect_physical[0].y() * aspect); else rect_physical[0].setX(0);

        ui->plainTextEditRect1X->setPlainText(QString("%1").arg(rect_physical[0].x(), 5, 'f', 3));
        set_other_Rect(0);
    }
}

void MainWindow::onSetRect2X()
{
    rect_physical[1].setX(ui->plainTextEditRect2X->toPlainText().toDouble());
    if (rect_physical[1].x() != 0){
        double aspect;
        normalize_rect_size();

        if (rect_draw[1].height() != 0) aspect = (double)rect_draw[1].width() / (double)rect_draw[1].height(); else aspect = 1;

        if (aspect != 0) rect_physical[1].setY(rect_physical[1].x() / aspect); else rect_physical[1].setY(0);

        ui->plainTextEditRect2Y->setPlainText(QString("%1").arg(rect_physical[1].y(), 5, 'f', 3));
        set_other_Rect(1);
    }
}

void MainWindow::onSetRect2Y()
{
    rect_physical[1].setY(ui->plainTextEditRect2Y->toPlainText().toDouble());
    if (rect_physical[1].y() != 0){
        double aspect;
        normalize_rect_size();

        if (rect_draw[1].height() != 0) aspect = (double)rect_draw[1].width() / (double)rect_draw[1].height(); else aspect = 1;

        if (aspect != 0) rect_physical[1].setX(rect_physical[1].y() * aspect); else rect_physical[1].setX(0);

        ui->plainTextEditRect2X->setPlainText(QString("%1").arg(rect_physical[1].x(), 5, 'f', 3));
        set_other_Rect(1);
    }
}
