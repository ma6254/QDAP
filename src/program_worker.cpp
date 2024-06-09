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

void ProgramWorker::read_chip(QByteArray *data)
{
    int32_t err;
    FlashDevice flash_info = algo->get_flash_device_info();
    uint32_t flash_size = flash_info.szDev;
    uint32_t flash_addr = flash_info.DevAdr;
    uint32_t page_size = flash_info.szPage;

    err = read(flash_addr, flash_size, data);
    if (err < 0)
    {
        qDebug("[ProgramWorker] exec_flash_func ReadChip fail");
        emit finished(ProgramWorker::ReadChip, false);
        return;
    }

    // data.fill(0x00, flash_size);
    // for (uint32_t i = 0; i < flash_size; i += page_size)
    // {
    //     err = dev->dap_read_memory(flash_addr + i, (uint8_t *)(data.data()) + i, page_size);
    //     if (err < 0)
    //     {
    //         qDebug("[ProgramWorker] exec_flash_func ReadChip fail");
    //         emit finished(ProgramWorker::ReadChip, false);
    //         return;
    //     }

    //     emit process(i, flash_size);
    // }

    emit finished(ProgramWorker::ReadChip, true);
}

int32_t ProgramWorker::read(uint32_t addr, uint32_t size, QByteArray *data, uint32_t page_size)
{
    int32_t err;
    uint32_t pr_size;

    data->clear();
    data->fill(0x00, size);

    for (uint32_t i = 0; i < size; i += page_size)
    {

        if ((i + page_size) > size)
        {
            pr_size = size % page_size;
        }
        else
        {
            pr_size = page_size;
        }

        err = dev->dap_read_memory(addr + i, (uint8_t *)(data->data()) + i, page_size);
        if (err < 0)
        {
            qDebug("[ProgramWorker] Read fail");
            // emit finished(ProgramWorker::ReadChip, false);
            return -1;
        }

        emit process(i, size);
    }

    return 0;
}

void ProgramWorker::write(uint32_t addr, QByteArray *data)
{
    int32_t err;
    program_syscall_t sys_call_s = algo->get_sys_call_s();
    FlashDevice flash_info = algo->get_flash_device_info();
    uint32_t flash_size = flash_info.szDev;
    uint32_t flash_addr = flash_info.DevAdr;
    uint32_t page_size = flash_info.szPage;
    uint32_t program_buff = algo->program_mem_buffer();

    uint32_t entry = algo->get_flash_func_offset(FLASH_FUNC_ProgramPage);
    if (entry == UINT32_MAX)
    {
        qDebug("[ProgramWorker] exec_flash_func unsuport func");
        emit finished(ProgramWorker::Program, false);
        return;
    }

    uint32_t fw_size = data->length();
    uint32_t pw_size;

    for (uint32_t i = 0; i < fw_size; i += page_size)
    {
        if ((i + page_size) > fw_size)
        {
            pw_size = fw_size % page_size;
        }
        else
        {
            pw_size = page_size;
        }

        err = dev->dap_write_memory(program_buff, (uint8_t *)(data->data()) + i, pw_size);
        if (err < 0)
        {
            qDebug("[ProgramWorker] exec_flash_func ReadChip fail");
            emit finished(ProgramWorker::ReadChip, false);
            return;
        }

        err = dev->swd_flash_syscall_exec(&sys_call_s, entry, flash_addr + i, pw_size, program_buff, 0);
        if (err < 0)
        {
            qDebug("[ProgramWorker] exec_flash_func ProgramPage fail");
            emit finished(ProgramWorker::Program, false);
            return;
        }

        emit process(i, fw_size);
    }

    emit finished(ProgramWorker::Program, true);
}

void ProgramWorker::verify(uint32_t addr, QByteArray *data)
{
    int32_t err;
    uint32_t pr_size;
    uint32_t page_size = 1024;
    QByteArray tmp_buf;

    tmp_buf.fill(0x00, page_size);

    for (uint32_t i = 0; i < data->length(); i += page_size)
    {
        if ((i + page_size) > data->length())
        {
            pr_size = data->length() % page_size;
        }
        else
        {
            pr_size = page_size;
        }

        err = dev->dap_read_memory(addr + i, (uint8_t *)(tmp_buf.data()), pr_size);
        if (err < 0)
        {
            qDebug("[ProgramWorker] Read fail");
            emit finished(ProgramWorker::Verify, false);
            return;
        }

        if (memcmp(tmp_buf.data(), data->data() + i, pr_size) != 0)
        {
            qDebug("[ProgramWorker] Verify fail addr:%d", i);
            emit finished(ProgramWorker::Verify, false);
            return;
        }

        emit process(i, data->length());
    }

    qDebug("[ProgramWorker] Verify ok");
    emit finished(ProgramWorker::Verify, true);
}
