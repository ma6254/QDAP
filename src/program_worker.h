#ifndef PROGRAM_WORKER_H
#define PROGRAM_WORKER_H

#include <QList>
#include <QThread>
#include <QDebug>
#include "dap_hid.h"
#include "flash_algo.h"

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
    void read_chip(void);
};

#endif // PROGRAM_WORK_H
