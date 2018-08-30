#include "videowidget.h"
#include "ui_videowidget.h"

VideoWidget::VideoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoWidget)
{
    saveDir = QDir::currentPath();
    ui->setupUi(this);
    connect(this, &VideoWidget::widgetPainted, ui->ui_frame_widget, &FrameWidget::onWidgetPainted);
}

VideoWidget::~VideoWidget()
{
    delete ui;
}

void VideoWidget::on_button_video_startcap_clicked()
{
    QByteArray arr;
    QDataStream s(&arr, QIODevice::WriteOnly);
    s << QString("startVideo");
    commandIssued(arr);

}

void VideoWidget::on_button_video_endcap_clicked()
{
    QByteArray arr;
    QDataStream s(&arr, QIODevice::WriteOnly);
    s << QString("stopVideo");
    commandIssued(arr);
}
void VideoWidget::onFramePresented(const QImage &image)
{
    emit widgetPainted(image);
}


void VideoWidget::on_button_video_saveimg_clicked()
{
    //拼接路径
    qDir.cd(saveDir);
//    cout << saveDir;
    if(qDir.exists(VIDEO_PATH) == false)
        qDir.mkdir(VIDEO_PATH);
    qDir.cd(VIDEO_PATH);

    QDateTime date =QDateTime::currentDateTime();
    QString name =qDir.absolutePath() + "/" + date.toString("yyyy-MM-dd-hh-mm") + ".jpg";
    emit imageSaved(name);
}
void VideoWidget::onSavePathChanged(const QString & name)
{
    saveDir = name;
}
