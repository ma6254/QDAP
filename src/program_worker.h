#ifndef PROGRAM_WORKER_H
#define PROGRAM_WORKER_H

#include <QList>
#include <QThread>
#include <QDebug>
#include "devices.h"
#include "flash_algo.h"
#include "utils.h"

class _ProgramWorker;

/*******************************************************************************
 * 封装一层
 ******************************************************************************/
class ProgramWorker : public QObject
{
    Q_OBJECT

public:
    ProgramWorker(DAP_HID *device, FlashAlgo *algo);
    ~ProgramWorker();

    typedef enum
    {
        EraseChip,
        ReadChip,
        EraseBlock,
        Program,
        Verify,
    } ChipOp;

private:
    DAP_HID *dev;
    FlashAlgo *algo;
    QThread *thread;

signals:
    void finished(ProgramWorker::ChipOp op, bool ok);
    void process(uint32_t val, uint32_t max);

public slots:
    void erase_chip(void);
    int32_t read(uint32_t addr, uint32_t size, QByteArray *data, uint32_t page_size = 1024);
    void read_chip(QByteArray *data);
    void write(uint32_t addr, QByteArray *data);
    void verify(uint32_t addr, QByteArray *data);
};

#endif // PROGRAM_WORK_H
