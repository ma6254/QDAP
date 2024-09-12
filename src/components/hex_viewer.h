#ifndef HEX_VIEWER_H
#define HEX_VIEWER_H

#include <QByteArray>
#include <QGridLayout>
#include <QRandomGenerator>
#include <QThread>
#include <QWidget>

namespace Ui
{
    class hex_viewer;
}

class HexViewerLoader : public QObject
{
    Q_OBJECT

public:
    HexViewerLoader(QGridLayout *parent_layout);
    ~HexViewerLoader();

public slots:
    void load(uint8_t *data, uint32_t len);

signals:
    void update_progress(uint32_t progress);
    void finished(QGridLayout *parent_layout);

private:
    QGridLayout *parent_layout;

    void clear_layout(void);
};

class HexViewer : public QWidget
{
    Q_OBJECT

public:
    explicit HexViewer(QWidget *parent = nullptr);
    ~HexViewer();

    void load(uint8_t *data, uint32_t size);
    void load(QByteArray data) { load((uint8_t *)(data.data()), data.size()); }
    void load(void) { load(NULL, 0); }

public slots:
    void loader_finished(QGridLayout *parent_layout);

signals:
    void loader_load(uint8_t *data, uint32_t len);

private:
    Ui::hex_viewer *ui;
    QGridLayout *parent_layout;
    QThread *thread_loader;
    HexViewerLoader *loader;

    void clear_layout(void);
    void add_title(void);
};

#endif // HEX_VIEWER_H
