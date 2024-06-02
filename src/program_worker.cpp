#include "program_worker.h"

ProgramWorker::ProgramWorker(DAP_HID *device, FlashAlgo *algo)
{
    this->dev = device;
    this->algo = algo;

    thread = new QThread();
    thread->start();

    // worker = new _ProgramWorker(dev, algo);
    // worker->moveToThread(thread);

    // connect(this, &ProgramWorker::_erase_chip, worker, &_ProgramWorker::erase_chip);
    // connect(worker, &_ProgramWorker::finished, this, &ProgramWorker::_finished);

    moveToThread(thread);
}

ProgramWorker::~ProgramWorker()
{
}

void ProgramWorker::erase_chip(void)
{
    int32_t err;
    program_syscall_t sys_call_s = algo->get_sys_call_s();

    uint32_t entry = algo->get_flash_func_offset(FLASH_FUNC_EraseChip);
    if (entry == UINT32_MAX)
    {
        qDebug("[ProgramWorker] exec_flash_func unsuport func");
        emit finished(ProgramWorker::EraseChip, false);
        return;
    }

    qDebug("[ProgramWorker] entry: 0x%08X", entry);

    err = dev->swd_flash_syscall_exec(&sys_call_s, entry, 0, 0, 0, 0);
    if (err < 0)
    {
        qDebug("[ProgramWorker] exec_flash_func EraseChip fail");
        emit finished(ProgramWorker::EraseChip, false);
        return;
    }

    emit finished(ProgramWorker::EraseChip, true);
}

void ProgramWorker::read_chip(void)
{
    int32_t err;

    FlashDevice flash_info = algo->get_flash_device_info();
    uint32_t flash_size = flash_info.szDev;
    uint32_t flash_addr = flash_info.DevAdr;

    QByteArray data;
    data.fill(0x00, flash_size);

    for (uint32_t i = 0; i < flash_size; i += 1024)
    {
        err = dev->dap_read_memory(flash_addr + i, (uint8_t *)(data.data()) + i, 1024);
        if (err < 0)
        {
            qDebug("[_ProgramWorker] exec_flash_func ReadChip fail");
            emit finished(ProgramWorker::ReadChip, false);
            return;
        }

        emit process(i, flash_size);
    }

    emit finished(ProgramWorker::ReadChip, true);
}
