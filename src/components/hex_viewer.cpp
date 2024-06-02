#include "hex_viewer.h"
#include "ui_hex_viewer.h"
#include "utils.h"
#include <QLabel>

bool is_visible_char(char a)
{
    // if ((a >= '0') && (a <= '9'))
    //     return true;
    // else if ((a >= 'a') && (a <= 'z'))
    //     return true;
    // else if ((a >= 'A') && (a <= 'Z'))
    //     return true;

    if ((a >= 0x20) && (a <= 0x7E))
        return true;

    return false;
}

HexViewer::HexViewer(QWidget *parent) : QWidget(parent),
                                        ui(new Ui::hex_viewer)
{
    ui->setupUi(this);

    parent_layout = (QGridLayout *)(ui->scrollAreaWidgetContents->layout());

    // thread_loader = new QThread();
    // loader = new HexViewerLoader(parent_layout);
    // connect(this, &HexViewer::loader_load, loader, &HexViewerLoader::load);
    // connect(loader, &HexViewerLoader::finished, this, &HexViewer::loader_finished);
    // loader->moveToThread(thread_loader);
    // thread_loader->start();

    add_title();
    // clear_layout();

    QByteArray init_data;
    init_data.fill(0x00, 42);
    // init_data.fill(0x00, 1024);

    qsrand(clock());

    for (uint32_t i = 0; i < init_data.length(); i++)
    {
        init_data[i] = char(qrand() % 256);
    }

    load(init_data);
}

HexViewer::~HexViewer()
{
    delete ui;
}

void HexViewer::clear_layout(void)
{
    uint32_t remove_count = 0;
    QLayoutItem *tmp_item;

    while ((tmp_item = parent_layout->takeAt(0)) != NULL)
    {
        if (tmp_item->widget() != NULL)
            delete tmp_item->widget();

        delete tmp_item;

        remove_count++;
    }

    // ui->scrollAreaWidgetContents->resize(0,0);

    qDebug("[HexViewer] clear_layout remove_count: %d", remove_count);
}

void HexViewer::add_title(void)
{
    uint32_t line_i = 0;
    QFont newFont_title("Courier New", 10, QFont::Bold, false);
    QLabel *tmp_label;

    tmp_label = new QLabel();
    tmp_label->setTextFormat(Qt::PlainText);
    tmp_label->setFont(newFont_title);
    tmp_label->setText(QString("Offset(h)"));
    tmp_label->setStyleSheet("QLabel{ color: blue;}");
    ui->gridLayout_title->addWidget(tmp_label, 0, 0);

    for (line_i = 0; line_i < 0x10; line_i++)
    {

        tmp_label = new QLabel();
        tmp_label->setTextFormat(Qt::PlainText);
        tmp_label->setFont(newFont_title);
        tmp_label->setText(QString(" "));
        tmp_label->setStyleSheet("QLabel{ color: blue;}");
        ui->gridLayout_title->addWidget(tmp_label, 0, 1UL + line_i * 2);

        tmp_label = new QLabel();
        tmp_label->setTextFormat(Qt::PlainText);
        tmp_label->setFont(newFont_title);
        tmp_label->setText(QString("%1").arg(line_i, 2, 16, QChar('0')).toUpper());
        tmp_label->setStyleSheet("QLabel{ color: blue;}");
        ui->gridLayout_title->addWidget(tmp_label, 0, 1UL + line_i * 2 + 1);
    }

    tmp_label = new QLabel();
    tmp_label->setTextFormat(Qt::PlainText);
    tmp_label->setFont(newFont_title);
    tmp_label->setText(QString(" "));
    tmp_label->setStyleSheet("QLabel{ color: blue;}");
    ui->gridLayout_title->addWidget(tmp_label, 0, 1UL + line_i * 2);

    tmp_label = new QLabel();
    tmp_label->setTextFormat(Qt::PlainText);
    tmp_label->setFont(newFont_title);
    tmp_label->setText(QString("对应文本"));
    tmp_label->setStyleSheet("QLabel{ color: blue;}");
    ui->gridLayout_title->addWidget(tmp_label, 0, 1ULL + 0x10 * 2 + 1);
    ui->gridLayout_title->setColumnStretch(1ULL + 0x10 * 2 + 1, 1);
}

// void HexViewer::load(uint8_t *data, uint32_t size)
// {
//     delete ui->scrollAreaWidgetContents->layout();

//     emit loader_load(data, size);
// }

void HexViewer::loader_finished(QGridLayout *parent_layout)
{
    qDebug("[HexViewer] loader_finished");

    ui->scrollAreaWidgetContents->setLayout(parent_layout);
}

void HexViewer::load(uint8_t *data, uint32_t size)
{
    uint32_t i = 0;
    uint32_t line_i = 0;
    QLabel *tmp_label;

    qDebug("[HexViewer] load: %d", size);
    hexdump(data, size);

    clear_layout();
    if (size == 0)
        return;

    uint32_t line_count = size / 0x10;
    if (size % 0x10)
        line_count++;

    qDebug("[HexViewer] rowCount:%d", parent_layout->rowCount());
    QFont newFont("Courier New", 10, QFont::Normal, false);
    QFont newFont_title("Courier New", 10, QFont::Bold, false);

    for (i = 0; i < size; i += 0x10)
    {
        uint32_t rowIndex = i / 0x10;

        QLabel *tmp_addr_label = new QLabel();
        tmp_addr_label->setTextFormat(Qt::PlainText);
        tmp_addr_label->setFont(newFont_title);
        tmp_addr_label->setText(QString("%1_%2").arg(i >> 16, 4, 16, QChar('0')).arg(i & 0xFFFF, 4, 16, QChar('0')).toUpper());
        tmp_addr_label->setStyleSheet("QLabel{ color: blue;}");
        parent_layout->addWidget(tmp_addr_label, rowIndex, 0);

        for (line_i = 0; line_i < 0x10; line_i++)
        {

            tmp_label = new QLabel();
            tmp_label->setTextFormat(Qt::PlainText);
            tmp_label->setFont(newFont);
            tmp_label->setText(QString(" "));
            parent_layout->addWidget(tmp_label, rowIndex, 1UL + line_i * 2);

            QLabel *tmp_byte_label = new QLabel();
            tmp_byte_label->setTextFormat(Qt::PlainText);
            tmp_byte_label->setFont(newFont);
            // tmp_byte_label->setAlignment(Qt::AlignCenter);
            // tmp_byte_label->setMouseTracking(true);

            parent_layout->addWidget(tmp_byte_label, rowIndex, 1UL + line_i * 2 + 1);

            if (line_i % 2)
            {
                // tmp_byte_label->setPa
                QPalette tmp_palette = tmp_byte_label->palette();
                tmp_palette.setColor(QPalette::WindowText, QColor(128, 96, 96));
                tmp_byte_label->setPalette(tmp_palette);
            }

            if ((i + line_i) >= size)
            {
                tmp_byte_label->setText("  ");
            }
            else
            {
                tmp_byte_label->setText(QString("%1").arg(data[i + line_i] & 0xFF, 2, 16, QChar('0')).toUpper());
            }

            //     tmp_byte_label->setText(QString(" %1").arg(*(data + i + line_i) & 0xFF, 2, 16, QChar('0')).toUpper());
            //     tmp_ascii_label->setText(QString("."));
            //     res.append(QString(" %1").arg(*(data + i + line_i) & 0xFF, 2, 16, QChar('0')).toUpper());
        }

        tmp_label = new QLabel();
        tmp_label->setTextFormat(Qt::PlainText);
        tmp_label->setFont(newFont);
        tmp_label->setText(QString(" "));
        parent_layout->addWidget(tmp_label, rowIndex, 1UL + 0x10 * 2);

        uint8_t lineIndexBase = 1UL + 0x10 * 2 + 1;

        for (line_i = 0; line_i < 0x10; line_i++)
        {
            QLabel *tmp_ascii_label = new QLabel();
            tmp_ascii_label->setTextFormat(Qt::PlainText);
            tmp_ascii_label->setFont(newFont);
            tmp_ascii_label->setAlignment(Qt::AlignCenter);

            if (is_visible_char(data[i + line_i]))
                tmp_ascii_label->setText(QChar(data[i + line_i]));
            else
                tmp_ascii_label->setText(QString("."));

            parent_layout->addWidget(tmp_ascii_label, rowIndex, lineIndexBase + line_i);
        }

        tmp_label = new QLabel();
        tmp_label->setTextFormat(Qt::PlainText);
        tmp_label->setFont(newFont);
        tmp_label->setText(QString(" "));
        tmp_label->setStyleSheet("QLabel{ color: blue;}");
        parent_layout->addWidget(tmp_label, rowIndex, 1UL + 0x10 * 3 + 1);
        parent_layout->setColumnStretch(1ULL + 0x10 * 3 + 1, 1);

        parent_layout->setRowStretch(rowIndex, 0);
    }

    tmp_label = new QLabel();
    tmp_label->setTextFormat(Qt::PlainText);
    tmp_label->setFont(newFont);
    tmp_label->setText(QString("--- 到底啦 --- "));
    tmp_label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    tmp_label->setStyleSheet("QLabel{ color: blue;}");
    parent_layout->addWidget(tmp_label, line_count, 0, 1, 1 + 0x10 * 3 + 1);
    parent_layout->setRowStretch(line_count, 1);

    // uint32_t title_width = ui->gridLayout_title->itemAtPosition(0, 0)->widget()->width();
    // uint32_t data_width = parent_layout->itemAtPosition(0, 0)->widget()->width();

    // qDebug("[HexViewer] load title_width:%d data_width:%d", title_width, data_width);

    // if (title_width > data_width)
    // {
    //     parent_layout->setColumnMinimumWidth(0, title_width);
    // }
    // else
    // {
    //     ui->gridLayout_title->setColumnMinimumWidth(0, data_width);
    // }
}

HexViewerLoader::HexViewerLoader(QGridLayout *parent_layout)
{
    this->parent_layout = parent_layout;
}

HexViewerLoader::~HexViewerLoader()
{
}

void HexViewerLoader::clear_layout(void)
{
    uint32_t remove_count = 0;
    QLayoutItem *tmp_item;

    while ((tmp_item = parent_layout->takeAt(0)) != NULL)
    {
        if (tmp_item->widget() != NULL)
            delete tmp_item->widget();

        delete tmp_item;

        remove_count++;
    }

    // ui->scrollAreaWidgetContents->resize(0,0);

    qDebug("[HexViewerLoader] clear_layout remove_count: %d", remove_count);
}

void HexViewerLoader::load(uint8_t *data, uint32_t len)
{
    uint32_t i = 0;
    uint32_t line_i = 0;
    QLabel *tmp_label;

    QGridLayout *parent_layout = new QGridLayout();

    qDebug("[HexViewerLoader] load: %d", len);
    // hexdump((uint8_t *)data, len);

    clear_layout();
    if (len == 0)
        return;

    uint32_t line_count = len / 0x10;
    if (len % 0x10)
        line_count++;

    qDebug("[HexViewerLoader] rowCount:%d", parent_layout->rowCount());
    QFont newFont("Courier New", 10, QFont::Normal, false);

    for (i = 0; i < len; i += 0x10)
    {
        uint32_t rowIndex = i / 0x10;

        QLabel *tmp_addr_label = new QLabel();
        tmp_addr_label->setTextFormat(Qt::PlainText);
        tmp_addr_label->setFont(newFont);
        tmp_addr_label->setText(QString("%1_%2").arg(i >> 16, 4, 16, QChar('0')).arg(i & 0xFFFF, 4, 16, QChar('0')).toUpper());
        tmp_addr_label->setStyleSheet("QLabel{ color: blue;}");
        parent_layout->addWidget(tmp_addr_label, rowIndex, 0);

        for (line_i = 0; line_i < 0x10; line_i++)
        {

            tmp_label = new QLabel();
            tmp_label->setTextFormat(Qt::PlainText);
            tmp_label->setFont(newFont);
            tmp_label->setText(QString(" "));
            parent_layout->addWidget(tmp_label, rowIndex, 1UL + line_i * 2);

            QLabel *tmp_byte_label = new QLabel();
            tmp_byte_label->setTextFormat(Qt::PlainText);
            tmp_byte_label->setFont(newFont);
            // tmp_byte_label->setAlignment(Qt::AlignCenter);
            // tmp_byte_label->setMouseTracking(true);

            parent_layout->addWidget(tmp_byte_label, rowIndex, 1UL + line_i * 2 + 1);

            if (line_i % 2)
            {
                // tmp_byte_label->setPa
                QPalette tmp_palette = tmp_byte_label->palette();
                tmp_palette.setColor(QPalette::WindowText, QColor(128, 96, 96));
                tmp_byte_label->setPalette(tmp_palette);
            }

            if ((i + line_i) >= len)
            {
                tmp_byte_label->setText("  ");
            }
            else
            {
                tmp_byte_label->setText(QString("%1").arg(data[i + line_i] & 0xFF, 2, 16, QChar('0')).toUpper());
            }

            //     tmp_byte_label->setText(QString(" %1").arg(*(data + i + line_i) & 0xFF, 2, 16, QChar('0')).toUpper());
            //     tmp_ascii_label->setText(QString("."));
            //     res.append(QString(" %1").arg(*(data + i + line_i) & 0xFF, 2, 16, QChar('0')).toUpper());
        }

        tmp_label = new QLabel();
        tmp_label->setTextFormat(Qt::PlainText);
        tmp_label->setFont(newFont);
        tmp_label->setText(QString(" "));
        parent_layout->addWidget(tmp_label, rowIndex, 1UL + 0x10 * 2);

        uint8_t lineIndexBase = 1UL + 0x10 * 2 + 1;

        for (line_i = 0; line_i < 0x10; line_i++)
        {
            QLabel *tmp_ascii_label = new QLabel();
            tmp_ascii_label->setTextFormat(Qt::PlainText);
            tmp_ascii_label->setFont(newFont);
            tmp_ascii_label->setAlignment(Qt::AlignCenter);

            if (is_visible_char(data[i + line_i]))
                tmp_ascii_label->setText(QString("%1").arg(data[i + line_i]));
            else
                tmp_ascii_label->setText(QString("."));

            parent_layout->addWidget(tmp_ascii_label, rowIndex, lineIndexBase + line_i);
        }

        tmp_label = new QLabel();
        tmp_label->setTextFormat(Qt::PlainText);
        tmp_label->setFont(newFont);
        tmp_label->setText(QString(" "));
        tmp_label->setStyleSheet("QLabel{ color: blue;}");
        parent_layout->addWidget(tmp_label, rowIndex, 1UL + 0x10 * 3 + 1);
        parent_layout->setColumnStretch(1ULL + 0x10 * 3 + 1, 1);

        parent_layout->setRowStretch(rowIndex, 0);
    }

    tmp_label = new QLabel();
    tmp_label->setTextFormat(Qt::PlainText);
    tmp_label->setFont(newFont);
    tmp_label->setText(QString("--- 到底啦 --- "));
    tmp_label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    tmp_label->setStyleSheet("QLabel{ color: blue;}");
    parent_layout->addWidget(tmp_label, line_count, 0, 1, 1 + 0x10 * 3 + 1);
    parent_layout->setRowStretch(line_count, 1);

    emit finished(parent_layout);
}
