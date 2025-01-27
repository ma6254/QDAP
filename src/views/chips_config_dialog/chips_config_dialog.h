#ifndef CHIPS_CONFIG_DIALOG_H
#define CHIPS_CONFIG_DIALOG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QFile>
#include <QUrl>

namespace Ui
{
    class chips_config_dialog;
}

class ChipsConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChipsConfigDialog(QWidget *parent = nullptr);
    ~ChipsConfigDialog();

    QString get_chips_url();
    void set_chips_url(QString url);

public slots:
    void cb_btn_ok();
    void lineEdit_url_textEdited(QString text);
    void cb_btn_refresh();
    void cb_btn_abort();

    void cb_reply_finished();
    void cb_reply_redirected(QUrl url);
    void cb_reply_sslErrors(QList<QSslError> errors);
    void cb_reply_errorOccurred(QNetworkReply::NetworkError code);
    void cb_reply_readyRead();
    void cb_reply_downloadProgress(qint64 bytesRead, qint64 totalBytes);

private:
    int http_get_request(QUrl url);
    int http_get_request(QString url) { return http_get_request(QUrl(url)); };
    int extract();
    int apply();

private:
    Ui::chips_config_dialog *ui;

    QByteArray file_buf;

    QNetworkAccessManager networkManager;
    QNetworkRequest request;
    QNetworkReply *reply;
};

#endif // DIALOG_CHIPS_CONFIG_H
