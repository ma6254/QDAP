#ifndef FLASHALGO_H
#define FLASHALGO_H

#include <QString>
#include <QList>
#include <QThread>
#include <QDebug>
#include "glibc_elf.h"
#include "FlashOS.h"

#define FUNC_SYMBOL_COUNT 6

typedef struct
{
    const char *name;
    uint8_t must;
} flash_func_def_t;

typedef enum
{
    FLASH_FUNC_Init = 0,
    FLASH_FUNC_UnInit,
    FLASH_FUNC_BlankCheck,
    FLASH_FUNC_EraseChip,
    FLASH_FUNC_EraseSector,
    FLASH_FUNC_ProgramPage,
    FLASH_FUNC_Verify,
    FLASH_FUNC_MAX,
} flash_func_t;

class FlashAlgo : public QObject
{
    Q_OBJECT

public:
    FlashAlgo();
    ~FlashAlgo();

    int32_t load(QString file_path);

private:
    QString section_get_name(Elf32_Shdr section);
    QString symbol_get_name(Elf32_Sym symbol);
    QString section_type_to_str(uint32_t section_type);
    QString section_flags_to_str(uint32_t section_flags);
    QString FlashDevice_type_to_str(uint16_t type);

    bool is_flash_func_impl(flash_func_t f);

    QByteArray file_buf;
    Elf32_Ehdr elf_header;
    Elf32_Shdr sh_str_section;
    Elf32_Shdr symbol_section;
    Elf32_Shdr str_section;
    Elf32_Shdr PrgCode_section;

    Elf32_Sym FlashDevice_symbol;
    Elf32_Sym flash_func_symbol_list[FUNC_SYMBOL_COUNT];
    QByteArray flash_func_code_list[FUNC_SYMBOL_COUNT];

    // Elf32_Sym Init_symbol;
    // Elf32_Sym UnInit_symbol;
    // Elf32_Sym BlankCheck_symbol;
    // Elf32_Sym EraseChip_symbol;
    // Elf32_Sym EraseSector_symbol;
    // Elf32_Sym ProgramPage_symbol;
    // Elf32_Sym Verify_symbol;

    FlashDevice flash_device_info;
};

#endif // FLASHALGO_H
