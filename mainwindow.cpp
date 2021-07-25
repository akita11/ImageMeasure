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
#include <QColorDialog>

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
    rotate = 0;
    dragMode = 0;
    pen_color[0] = Qt::red;
    pen_color[1] = Qt::blue;
    QPalette pal;
    pal.setColor(QPalette::Button, pen_color[0]);
    ui->pushButtonRect1Color->setPalette(pal);
    pal.setColor(QPalette::Button, pen_color[1]);
    ui->pushButtonRect2Color->setPalette(pal);
/*
    imageFile = QImage("/Users/akita/Desktop/VHX_000218a.jpg");
    fImageLoaded = true;
    ui->SliderZoom->setEnabled(true);
    mag = 2;
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
//        img = img.scaled(imageWidth * mag, imageHeight * mag, Qt::KeepAspectRatio, Qt::SmoothTransformation);

//        for (int i = 0; i < 100; i++) img.setPixel(i, i, pen_color[0]);

        // painterの拡大縮小変換
//        QTransform matrix;
//        matrix.scale(1, 1);
//        painter.setWorldTransform(matrix);

        // QImageの回転などの変換
        // shttps://base64.work/so/qt/1908471
        QPoint center = img.rect().center();
        QTransform matrix;
        matrix.scale(mag, mag);
        matrix.translate(center.x(), center.y());
        matrix.rotate(rotate);
        img = img.transformed(matrix);

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
                        +-Y-+     hScroll=0.5
                        ^  +-Y-+  hScroll=1.0
                        posX
         */

        qDebug("%d %d / %d %d / %.1f / %d %d / I(%d %d)-(%d %d) D(%d %d)-(%d %d)",
               drawingWidth, drawingHeight, imageWidth, imageHeight, mag, posX, posY,
               rect_draw[0].left(), rect_draw[0].top(), rect_draw[0].right(), rect_draw[0].bottom(),
               conv_image_to_drawing(rect_draw[0]).left(),
                conv_image_to_drawing(rect_draw[0]).top(),
                conv_image_to_drawing(rect_draw[0]).right(),
                conv_image_to_drawing(rect_draw[0]).bottom());
    }
    painter.setPen(pen_color[0]);
//    painter.drawRect(rect_draw[0]);
    painter.drawRect(conv_image_to_drawing(rect_draw[0]));
    painter.setPen(pen_color[1]);
//    painter.drawRect(rect_draw[1]);
    painter.drawRect(conv_image_to_drawing(rect_draw[1]));
}

// 矩形を描画後に拡大縮小するときの矩形の拡大縮小がどうもうまくいかないので，とりあえず矩形は拡大縮小しない

/*
<--W-->     <--W*mag-->    W=imageWidth, Y=drawingWidth
+-----+  -> +---------+
               +-Y-+
               posX

            <----W*mag2---->
         -> +--------------+
                 +-Y-+
                 posX

*/

// xd@Drawing(Y) <-> xi@Image(W*mag)
//   -> xi = posX + xd / Y * (W*mag)
//   -> xd = (xi - posX) * Y / (W*mag)
QRect MainWindow::conv_drawing_to_image(QRect p)
{
    QRect pt;
/*
    pt.setLeft(posX + p.x() / (double)drawingWidth * (double)(imageWidth * mag));
    pt.setRight(pt.left() + imageWidth);
    pt.setTop(posY + p.y() / (double)drawingHeight * (double)(imageHeight * mag));
    pt.setBottom(pt.top() + imageHeight);
*/
    pt.setTopLeft(conv_drawing_to_image_p(p.topLeft()));
    pt.setBottomRight(conv_drawing_to_image_p(p.bottomRight()));
    return(pt);
}

QRect MainWindow::conv_image_to_drawing(QRect p)
{
    QRect pt;
/*
    pt.setLeft((p.x() - posX) * (double)drawingWidth / (double)(imageWidth * mag));
    pt.setRight(pt.left() + drawingWidth);
    pt.setTop((p.y() - posY) * (double)drawingHeight / (double)(imageHeight * mag));
    pt.setBottom(pt.top() + drawingHeight);
*/
    pt.setTopLeft(conv_image_to_drawing_p(p.topLeft()));
    pt.setBottomRight(conv_image_to_drawing_p(p.bottomRight()));
    return(pt);
}

QPoint MainWindow::conv_drawing_to_image_p(QPoint p)
{
    QPoint pt;
    pt.setX(posX + p.x());
    pt.setY(posY + p.y());
//    pt.setX(posX + p.x() / (double)drawingWidth * (double)(imageWidth * mag));
//    pt.setY(posY + p.y() / (double)drawingHeight * (double)(imageHeight * mag));
//    pt.setX(posX + p.x() * (double)drawingWidth / (double)(imageWidth * mag));
//    pt.setY(posY + p.y() * (double)drawingHeight / (double)(imageHeight * mag));
    return(pt);
}

QPoint MainWindow::conv_image_to_drawing_p(QPoint p)
{
    QPoint pt;
    pt.setX(p.x() - posX);
    pt.setY(p.y() - posY);
//    pt.setX((p.x() - posX) * (double)drawingWidth / (double)(imageWidth * mag));
//    pt.setY((p.y() - posY) * (double)drawingHeight / (double)(imageHeight * mag));
//    pt.setX((p.x() - posX) / (double)drawingWidth * (double)(imageWidth * mag));
//    pt.setY((p.y() - posY) / (double)drawingHeight * (double)(imageHeight * mag));
    return(pt);
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

double MainWindow::distance(QPoint p1, QPoint p2)
{
    return(sqrt((double)(p1.x() - p2.x()) * (p1.x() - p2.x()) + (double)(p1.y() - p2.y()) * (p1.y() - p2.y())));
}

bool MainWindow::check_within(int x, int xs, int xe)
{
    if (xs > xe){
        int xt = xe; xe = xs; xs = xt;
    }
    if (x >= xs && x <= xe) return(true); else return(false);
}


#define TH_DRAG 5
// マウスのイベントの取得(Qt4とQt5でぜんぜん違う）
// http://dorafop.my.coocan.jp/Qt/Qt104.html
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int drawingHeight = ui->horizontalLayout->geometry().height();
        if (event->position().y() < drawingHeight){
            //        qDebug("MousePress x=%f,y=%f", event->position().x(), event->position().y());
            QPoint mouseP = conv_drawing_to_image_p(event->position().toPoint());
            dragMode = 0;
            for (int i = 0; i < 2; i++){
                int dL, dR, dT, dB;
                dL = abs(mouseP.x() - rect_draw[i].left());
                dR = abs(mouseP.x() - rect_draw[i].right());
                dT = abs(mouseP.y() - rect_draw[i].top());
                dB = abs(mouseP.y() - rect_draw[i].bottom());
//                qDebug("%d : (%d %d) / (%d %d) - (%d %d) / %d %d %d %d", i, mouseP.x(), mouseP.y(), rect_draw[i].left(), rect_draw[i].top(), rect_draw[i].right(), rect_draw[i].bottom(), dL, dR, dT, dB);
                if (dL < TH_DRAG && check_within(mouseP.y(), rect_draw[i].bottom(), rect_draw[i].top())) dragMode = 1 + i * 10;
                else if (dR < TH_DRAG && check_within(mouseP.y(), rect_draw[i].bottom(), rect_draw[i].top())) dragMode = 2 + i * 10;
                else if (dT < TH_DRAG && check_within(mouseP.x(), rect_draw[i].left(), rect_draw[i].right())) dragMode = 3 + i * 10;
                else if (dB < TH_DRAG && check_within(mouseP.x(), rect_draw[i].left(), rect_draw[i].right())) dragMode = 4 + i * 10;
            }
            if (dragMode == 0) rect_draw[index].setTopLeft(mouseP);
            qDebug("%d p[%d %d] m[%f %f] D[%d %d] r[%d %d]", dragMode,
                   posX, posY,
                   event->position().x(), event->position().y(),
                   mouseP.x(), mouseP.y(),
                   rect_draw[index].left(), rect_draw[index].top());
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int drawingHeight = ui->horizontalLayout->geometry().height();
        if (event->position().y() < drawingHeight){
//            if (drawMode == 0) rect_draw[index].setBottomRight(event->position().toPoint());
        }
//        qDebug("MouseRelease x=%f,y=%f", event->position().x(), event->position().y());
        dragMode = 0;
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
    QPoint mouseP = conv_drawing_to_image_p(event->position().toPoint());

    if (dragMode == 0) rect_draw[index].setBottomRight(mouseP);
    else if (dragMode == 1) rect_draw[0].setLeft(mouseP.x());
    else if (dragMode == 2) rect_draw[0].setRight(mouseP.x());
    else if (dragMode == 3) rect_draw[0].setTop(mouseP.y());
    else if (dragMode == 4) rect_draw[0].setBottom(mouseP.y());
    else if (dragMode == 11) rect_draw[1].setLeft(mouseP.x());
    else if (dragMode == 12) rect_draw[1].setRight(mouseP.x());
    else if (dragMode == 13) rect_draw[1].setTop(mouseP.y());
    else if (dragMode == 14) rect_draw[1].setBottom(mouseP.y());

    int width = rect_draw[index].right() - rect_draw[index].left() + 1; if (width < 0) width = -width;
    int height = rect_draw[index].bottom() - rect_draw[index].top() + 1; if (height < 0) height = -height;
    // note: width = xe - xs + 1
//    qDebug("%d : %d %d %d %d / %d %d / %d %d", index, rect_draw[index].left(), rect_draw[index].top(), rect_draw[index].right(), rect_draw[index].bottom(), rect_draw[index].width(), rect_draw[index].height(), width, height);
    if (index == 0){
        ui->radioButtonRect1->setText((QString("Rect1(%1 %2)").arg(width).arg(height)));
    }
    else{
        ui->radioButtonRect2->setText((QString("Rect2(%1 %2)").arg(width).arg(height)));
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
        normalize_rect_size();
/*
        double aspect;
        if (rect_draw[0].height() != 0) aspect = (double)rect_draw[0].width() / (double)rect_draw[0].height(); else aspect = 1;
        if (aspect != 0) rect_physical[0].setY(rect_physical[0].x() / aspect); else rect_physical[0].setY(0);
*/
        if (rect_draw[0].width() != 0) rect_physical[0].setY(((double)rect_physical[0].x() * (double)rect_draw[0].height() / (double)rect_draw[0].width()));
        else rect_physical[0].setY(0);
//        qDebug("%d %d %d %d", rect_draw[0].width(), rect_draw[0].height(), rect_physical[0].x(), rect_physical[0].y());
        ui->plainTextEditRect1Y->setPlainText(QString("%1").arg(rect_physical[0].y(), 5, 'f', 3));
        set_other_Rect(0);
    }
}

void MainWindow::onSetRect1Y()
{
    rect_physical[0].setY(ui->plainTextEditRect1Y->toPlainText().toDouble());
    if (rect_physical[0].y() != 0){
        normalize_rect_size();
/*
        double aspect;
        if (rect_draw[0].height() != 0) aspect = (double)rect_draw[0].width() / (double)rect_draw[0].height(); else aspect = 1;
        if (aspect != 0) rect_physical[0].setX(rect_physical[0].y() * aspect); else rect_physical[0].setX(0);
*/
        if (rect_draw[0].height() != 0) rect_physical[0].setX(((double)rect_physical[0].y() * (double)rect_draw[0].width() / (double)rect_draw[0].height()));
        else rect_physical[0].setX(0);

        ui->plainTextEditRect1X->setPlainText(QString("%1").arg(rect_physical[0].x(), 5, 'f', 3));
        set_other_Rect(0);
    }
}

void MainWindow::onSetRect2X()
{
    rect_physical[1].setX(ui->plainTextEditRect2X->toPlainText().toDouble());
    if (rect_physical[1].x() != 0){
        normalize_rect_size();
/*
        double aspect;
        if (rect_draw[1].height() != 0) aspect = (double)rect_draw[1].width() / (double)rect_draw[1].height(); else aspect = 1;
        if (aspect != 0) rect_physical[1].setY(rect_physical[1].x() / aspect); else rect_physical[1].setY(0);
*/
        if (rect_draw[1].width() != 0) rect_physical[1].setY(((double)rect_physical[1].x() * (double)rect_draw[1].height() / (double)rect_draw[1].width()));
        else rect_physical[1].setY(0);

        ui->plainTextEditRect2Y->setPlainText(QString("%1").arg(rect_physical[1].y(), 5, 'f', 3));
        set_other_Rect(1);
    }
}

void MainWindow::onSetRect2Y()
{
    rect_physical[1].setY(ui->plainTextEditRect2Y->toPlainText().toDouble());
    if (rect_physical[1].y() != 0){
        normalize_rect_size();
/*
        double aspect;
        if (rect_draw[1].height() != 0) aspect = (double)rect_draw[1].width() / (double)rect_draw[1].height(); else aspect = 1;
        if (aspect != 0) rect_physical[1].setX(rect_physical[1].y() * aspect); else rect_physical[1].setX(0);
*/
        if (rect_draw[1].height() != 0) rect_physical[1].setX(((double)rect_physical[1].y() * (double)rect_draw[1].width() / (double)rect_draw[1].height()));
        else rect_physical[1].setX(0);

        ui->plainTextEditRect2X->setPlainText(QString("%1").arg(rect_physical[1].x(), 5, 'f', 3));
        set_other_Rect(1);
    }
}

void MainWindow::onRotateChanged()
{
    rotate = ui->plainTextEditRotate->toPlainText().toDouble();
    update();
}

void MainWindow::onRect1ColorClicked()
{
    QColor color = QColorDialog::getColor();
    if(color.isValid()){
        pen_color[0] = color;
        QPalette pal;
        pal.setColor(QPalette::Button, color);
        ui->pushButtonRect1Color->setPalette(pal);
    }
}

void MainWindow::onRect2ColorClicked()
{
    QColor color = QColorDialog::getColor();
    if(color.isValid()){
        pen_color[1] = color;
        QPalette pal;
        pal.setColor(QPalette::Button, color);
        ui->pushButtonRect2Color->setPalette(pal);
    }
}
