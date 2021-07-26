#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QPainter>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    // sceneはここで宣言
    // https://sites.google.com/site/drtomotomos/development/qt/displayimageonform
    QGraphicsScene scene;
    QImage imageFile, imageDraw;
    QGraphicsPixmapItem *imageFile_item;
    double mag = 1.0;
    int posX, posY;
    bool fImageLoaded = false;
    double hScroll, vScroll;
    int index;
    QRect rect_draw[2];
    QPointF rect_physical[2];
    void set_other_Rect(int index);
    void normalize_rect_size();
    double rotate;
    double distance(QPoint p1, QPoint p2);
    int dragMode;
    bool check_within(int x, int xs, int xe);
    QColor pen_color[2];
    QRect conv_drawing_to_image(QRect p);
    QRect conv_image_to_drawing(QRect p);
    QPoint conv_drawing_to_image_p(QPoint p);
    QPoint conv_image_to_drawing_p(QPoint p);
    int imageWidth, imageHeight, drawingWidth, drawingHeight;
    int Nrect = 2;

public slots:
    // 独自のスロット（ボタンを押したときに呼ばれる関数など）はここで宣言
    // https://www.qt.io/ja-jp/blog/2011/03/23/go-to-slot
    // スロット関数をここで追加しても，Designerのslotには出てこないので，編集→＋で追加
    // このとき関数名が違っていると，実行時にwarningは出る（エラーは出ない）
    // ウイジェットから右クリック→「スロットへ移動」でも関数をつくれる（関数名は決め打ちになるが，このほうが簡単）
    // https://y-nsk.hatenablog.com/entry/color-button


    void OpenFileDialog();
    void onSliderZoomChanged(int value);
    void onRadiobuttonClicked();
    void onHorizontalScrollBarChanged(int value);
    void onVerticalScrollBarChanged(int value);
    void onSetRect1X();
    void onSetRect1Y();
    void onSetRect2X();
    void onSetRect2Y();
    void onRotateChanged();
    void onRect1ColorClicked();
    void onRect2ColorClicked();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *event);
};
#endif // MAINWINDOW_H
