#include "chips_config_dialog.h"
#include "ui_chips_config_dialog.h"
#include <QDir>
#include <QFileInfo>
#include <QNetworkProxy>
#include <QtGui/private/qzipreader_p.h>
#include <QtGui/private/qzipwriter_p.h>

static const QString default_chips_url = "https://github.com/ma6254/qdap_chips/archive/refs/heads/main.zip";
static const QString tmp_chips_compress_file = ".tmp/qdap_chips-main.zip";
static const QString tmp_chips_directory = ".tmp/chips";
static const QString chips_directory = "chips";

/*******************************************************************************
 * @brief 器件库配置对话窗口，构造函数
 * @param parent 父组件
 * @return None
 ******************************************************************************/
ChipsConfigDialog::ChipsConfigDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::chips_config_dialog)
{
    ui->setupUi(this);

    reply = NULL;

    qDebug("[SSL] supports: %d", QSslSocket::supportsSsl());
    qDebug("[SSL] sslLibraryBuildVersion: %s", qPrintable(QSslSocket::sslLibraryBuildVersionString()));
    qDebug("[SSL] sslLibraryVersion: %s", qPrintable(QSslSocket::sslLibraryVersionString()));

    connect(ui->btn_ok, SIGNAL(clicked()), this, SLOT(cb_btn_ok()));
    connect(ui->btn_refresh, SIGNAL(clicked()), this, SLOT(cb_btn_refresh()));
    connect(ui->btn_abort, SIGNAL(clicked()), this, SLOT(cb_btn_abort()));
    connect(ui->lineEdit_chips_url, SIGNAL(textEdited(QString)), this, SLOT(lineEdit_url_textEdited(QString)));

    // 可以识别到系统代理，可以不需要手动设置
    // QNetworkProxy proxy;
    // proxy.setType(QNetworkProxy::HttpProxy);
    // proxy.setHostName("127.0.0.1");
    // proxy.setPort(10809);
    // networkManager.setProxy(proxy);

    // cb_btn_refresh();

    ui->label_msg->setText("未验证");
    ui->progressBar->setVisible(false);
}

/*******************************************************************************
 * @brief 器件库配置对话窗口，析构函数
 * @param None
 * @return None
 ******************************************************************************/
ChipsConfigDialog::~ChipsConfigDialog()
{
    delete ui;

    if (reply)
    {
        disconnect(reply, nullptr, nullptr, nullptr);
        reply->abort();
        reply->deleteLater();
        delete reply;
    }
}

/*******************************************************************************
 * @brief 回调函数，确认按钮按下
 * @param None
 * @return None
 ******************************************************************************/
void ChipsConfigDialog::cb_btn_ok()
{
    qDebug("[ChipsConfigDialog] cb_btn_ok");

    QDir tempDir(chips_directory);
    if (tempDir.exists())
    {
        tempDir.removeRecursively();
    }
    QDir().rename(tmp_chips_directory, chips_directory);

    emit accept();
}

/*******************************************************************************
 * @brief 回调函数，中止按钮按下
 * @param None
 * @return None
 ******************************************************************************/
void ChipsConfigDialog::cb_btn_abort()
{
    qDebug("[ChipsConfigDialog] cb_btn_abort");

    if (reply)
    {

        disconnect(reply, nullptr, nullptr, nullptr);
        reply->abort();
        reply->deleteLater();
        delete reply;
        reply = NULL;
    }
}

void ChipsConfigDialog::lineEdit_url_textEdited(QString text)
{
    qDebug("[ChipsConfigDialog] url_textEdited %s", qPrintable(text));

    ui->label_msg->setText("未验证");
    ui->btn_ok->setEnabled(false);
}

/*******************************************************************************
 * @brief 回调函数，刷新按钮按下
 * @param None
 * @return None
 ******************************************************************************/
void ChipsConfigDialog::cb_btn_refresh()
{
    qDebug("[ChipsConfigDialog] cb_btn_refresh");
    ui->btn_refresh->setEnabled(false);
    ui->lineEdit_chips_url->setEnabled(false);

    QUrl newUrl = QUrl::fromUserInput(get_chips_url());
    qDebug("[ChipsConfigDialog] url: %s", qPrintable(newUrl.toString()));

    // networkManager.setNetworkAccessible(QNetworkAccessManager::Accessible);

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(true);

    // reply->ignoreSslErrors();

    ui->label_msg->setText("发送了消息，等待服务器回复");
    http_get_request(newUrl);
    ui->btn_ok->setEnabled(false);
}

/*******************************************************************************
 * @brief 回调函数，HTTP完成函数
 * @param None
 * @return None
 ******************************************************************************/
void ChipsConfigDialog::cb_reply_finished()
{
    int err;

    ui->progressBar->setVisible(false);
    ui->btn_refresh->setEnabled(true);
    ui->lineEdit_chips_url->setEnabled(true);
    qDebug("[ChipsConfigDialog] finished");

    QDir(".tmp").mkdir(".");
    QFile *downloadedFile = new QFile(tmp_chips_compress_file);
    if (downloadedFile->open(QIODevice::WriteOnly) == false)
    {
        QMessageBox::information(this, tr("错误"), "临时文件打开错误");
        return;
    }

    file_buf.append(reply->readAll());
    downloadedFile->write(file_buf);
    downloadedFile->close();
    delete downloadedFile;
    downloadedFile = Q_NULLPTR;

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug("[ChipsConfigDialog] finished statusCode %d", statusCode);

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug("[ChipsConfigDialog] finished err: %d %s", reply->error(), qPrintable(reply->errorString()));

        ui->label_msg->setText(QString("通讯失败：%1").arg(qPrintable(reply->errorString())));

        reply->deleteLater(); //
        reply = Q_NULLPTR;
        return;
    }

    if (file_buf.count() == 0)
    {
        qDebug("[ChipsConfigDialog] finished get empty");

        reply->deleteLater();
        reply = Q_NULLPTR;
        return;
    }

    err = extract();
    if (err < 0)
    {
        ui->label_msg->setText("解压失败");
        return;
    }

    ui->label_msg->setText("成功");
    ui->progressBar->setVisible(true);
    ui->btn_ok->setEnabled(true);
}

void ChipsConfigDialog::cb_reply_redirected(QUrl url)
{
    qDebug("[ChipsConfigDialog] cb_reply_redirected %s", qPrintable(url.toString()));
}

void ChipsConfigDialog::cb_reply_sslErrors(QList<QSslError> errors)
{
    qDebug("[ChipsConfigDialog] cb_reply_sslErrors count:%d", errors.count());
}

void ChipsConfigDialog::cb_reply_errorOccurred(QNetworkReply::NetworkError code)
{
    qDebug("[ChipsConfigDialog] cb_reply_errorOccurred %d %s", code, qPrintable(QVariant::fromValue(code).toString()));
}

void ChipsConfigDialog::cb_reply_readyRead()
{
    // qDebug("[ChipsConfigDialog] reply_readyRead");
    file_buf.append(reply->readAll());
}

void ChipsConfigDialog::cb_reply_downloadProgress(qint64 bytesRead, qint64 totalBytes)
{
    // qDebug("[ChipsConfigDialog] download_progress");
    qDebug("[ChipsConfigDialog] download_progress %d/%d", bytesRead, totalBytes);
    // qDebug("[ChipsConfigDialog] download_progress ContentLengthHeader: %s", qPrintable(reply->header(QNetworkRequest::ContentLengthHeader).toString()));

    if (totalBytes <= 0)
    {
        ui->progressBar->setMinimum(0);
        ui->progressBar->setMaximum(0);
        ui->progressBar->setValue(bytesRead);
    }
    else
    {
        ui->progressBar->setMinimum(0);
        ui->progressBar->setMaximum(totalBytes);
        ui->progressBar->setValue(bytesRead);
    }
}

QString ChipsConfigDialog::get_chips_url()
{
    QString tmp_str = ui->lineEdit_chips_url->text();

    if (tmp_str.isEmpty())
    {
        ui->lineEdit_chips_url->setText(default_chips_url);
    }

    return ui->lineEdit_chips_url->text();
}

void ChipsConfigDialog::set_chips_url(QString url)
{
    if (url.isEmpty())
    {
        url = default_chips_url;
    }

    ui->lineEdit_chips_url->setText(url);
}

int ChipsConfigDialog::http_get_request(QUrl url)
{
    request.setUrl(url);
    // request.setRawHeader("User-Agent", "QDAP/1.0");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36");
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    file_buf.clear();
    reply = networkManager.get(request);

    file_buf.clear();
    reply = networkManager.get(request);

    connect(reply, SIGNAL(finished()), this, SLOT(cb_reply_finished()));
    connect(reply, SIGNAL(readyRead()), this, SLOT(cb_reply_readyRead()));

    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(cb_reply_sslErrors(QList<QSslError>)));

    connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)),
            this, SLOT(cb_reply_errorOccurred(QNetworkReply::NetworkError)));

    connect(reply, SIGNAL(redirected(QUrl)),
            this, SLOT(cb_reply_redirected(QUrl)));

    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(cb_reply_downloadProgress(qint64, qint64)));

    return 0;
}

int ChipsConfigDialog::extract()
{

    QDir tempDir(tmp_chips_directory);
    if (tempDir.exists())
    {
        tempDir.removeRecursively();
    }
    tempDir.mkdir(".");
    QZipReader reader(tmp_chips_compress_file);
    bool ok = reader.extractAll(tmp_chips_directory);
    if (ok == false)
    {
        return -1;
    }
    qDebug("[ChipsConfigDialog] extract");

    QStringList dir_list;
    QStringList file_list;

    QDir dir(tmp_chips_directory);
    // QString chips_directory = dir.fromNativeSeparators(chips_directory);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
    dir_list = dir.entryList();

    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
    file_list = dir.entryList();

    qDebug("[ChipsConfigDialog] extract dir_list len:%d", dir_list.length());
    qDebug("[ChipsConfigDialog] extract file_list len:%d", file_list.length());

    for (int i = 0; i < dir_list.length(); i++)
    {
        qDebug("[ChipsConfigDialog] extract ls %d %s", i, qPrintable(dir_list[i]));
    }

    if ((dir_list.length() == 1) && (file_list.length() == 0))
    {

        QDir(tmp_chips_directory + "1").removeRecursively();
        QDir(".").rename(tmp_chips_directory + QDir::separator() + dir_list[0], tmp_chips_directory + "1");
        QDir(tmp_chips_directory).removeRecursively();
        QDir().rename(tmp_chips_directory + "1", tmp_chips_directory);
    }

    qDebug("[ChipsConfigDialog] extract finished");

    // 网络响应结束
    // QFileInfo fileInfo;
    // fileInfo.setFile(downloadedFile->fileName());

    return 0;
}

int ChipsConfigDialog::apply()
{
    return 0;
}
