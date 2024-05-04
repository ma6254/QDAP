#include "flash_algo.h"
#include <QFile>

flash_func_def_t func_symbol_list[FUNC_SYMBOL_COUNT] = {
    {
        .name = "Init",
        .must = 1,
    },
    {.name = "UnInit", .must = 1},
    {.name = "BlankCheck", .must = 0},
    {.name = "EraseChip", .must = 1},
    {.name = "EraseSector", .must = 1},
    {.name = "ProgramPage", .must = 1},
    // "Verify",
};

FlashAlgo::FlashAlgo()
{
}

FlashAlgo::~FlashAlgo(void)
{
}

int32_t FlashAlgo::load(QString file_path)
{
    qDebug("[FlashAlgo] Load ===================================================");

    QFile elf_file(file_path);
    if (!elf_file.open(QIODevice::ReadOnly))
        return -1;

    file_buf = elf_file.readAll();
    elf_file.close();

    memcpy(&elf_header, file_buf.data(), sizeof(elf_header));

    if ((memcmp(elf_header.e_ident, ELFMAG, SELFMAG) != 0) &&
        (elf_header.e_type == 0x7f))
    {
        qDebug("[main] not ELF file");
        return -1;
    }

    // qDebug("[main] read elf ok");
    // qDebug("[main] e_shoff: 0x%08X", elf_header.e_shoff);         // Section Header Table 在文件中的偏移
    // qDebug("[main] e_shentsize: 0x%08X", elf_header.e_shentsize); // 单个Section Header大小
    // qDebug("[main] e_shnum: 0x%08X", elf_header.e_shnum);         // Section Header的数量
    // qDebug("[main] e_shstrndx: 0x%08X", elf_header.e_shstrndx);   // Section Header字符串表在Section Header Table中的索引

    memcpy(&sh_str_section, file_buf.data() + elf_header.e_shoff + sizeof(Elf32_Shdr) * elf_header.e_shstrndx, sizeof(Elf32_Shdr));
    // qDebug("[main] str_shdrs: 0x%08X", str_shdrs.sh_name);
    // qDebug("[main] str_sh_offset: 0x%08X", str_shdrs.sh_offset);

    QString str_section_name = section_get_name(sh_str_section);
    if (str_section_name != ".shstrtab")
    {
        qDebug("[FlashAlgo] not found str_tabel_section %s", qPrintable(str_section_name));
        return -1;
    }

    bool is_find_sym_section = false;
    bool is_find_str_section = false;
    bool is_find_PrgCode_section = false;

    for (int i = 0; i < elf_header.e_shnum; i++)
    {
        Elf32_Shdr shdrs;
        memcpy(&shdrs, file_buf.data() + elf_header.e_shoff + sizeof(Elf32_Shdr) * i, sizeof(Elf32_Shdr));

        QString section_name = section_get_name(shdrs);

        if ((section_name == ".symtab") && (shdrs.sh_type == SHT_SYMTAB))
        {
            symbol_section = shdrs;
            is_find_sym_section = true;
        }
        else if ((section_name == ".strtab") && (shdrs.sh_type == SHT_STRTAB))
        {
            str_section = shdrs;
            is_find_str_section = true;
        }
        else if (section_name == "PrgCode")
        {
            PrgCode_section = shdrs;
            is_find_PrgCode_section = true;
        }

        // qDebug("[main] ==== %d Section =========================================", i);
        // qDebug("    sh_name: 0x%08X %s", shdrs.sh_name, file_buf.data() + str_shdrs.sh_offset + shdrs.sh_name);
        // qDebug("    sh_offset: 0x%08X", shdrs.sh_offset);
        // qDebug("    sh_type: 0x%08X %s", shdrs.sh_type, section_type_to_str(shdrs.sh_type));

        // qDebug("    %d %s %s %s addr:0x%08X",
        //        i,
        //        qPrintable(section_name),
        //        qPrintable(section_type_to_str(shdrs.sh_type)),
        //        qPrintable(section_flags_to_str(shdrs.sh_flags)),
        //        shdrs.sh_offset);
    }

    if (is_find_sym_section == false)
    {
        qDebug("[FlashAlgo] not found symbol_tabel_section");
        return -1;
    }

    if (is_find_str_section == false)
    {
        qDebug("[FlashAlgo] not found str_tabel_section");
        return -1;
    }

    if (is_find_PrgCode_section == false)
    {
        qDebug("[FlashAlgo] not found PrgCode_section");
        return -1;
    }

    bool is_find_FlashDevice_symbol = false;
    bool is_find_Init_symbol[FUNC_SYMBOL_COUNT];

    memset(is_find_Init_symbol, false, sizeof(is_find_Init_symbol));

    // qDebug("[main] Symbol ======================================================");

    for (uint32_t start_addr = 0; start_addr < symbol_section.sh_size; start_addr += sizeof(Elf32_Sym))
    {
        Elf32_Sym symbol;
        memcpy(&symbol, file_buf.data() + symbol_section.sh_offset + start_addr, sizeof(Elf32_Sym));

        QString symbol_name = symbol_get_name(symbol);

        if (symbol_name == "FlashDevice")
        {
            FlashDevice_symbol = symbol;
            is_find_FlashDevice_symbol = true;
        }

        for (uint32_t i = 0; i < FUNC_SYMBOL_COUNT; i++)
        {
            if (symbol_name == func_symbol_list[i].name)
            {
                flash_func_symbol_list[i] = symbol;
                is_find_Init_symbol[i] = true;
            }
        }

        // uint32_t str_name_len = strlen(pstr);
        // QByteArray symbol_name_buf(pstr, str_name_len);
        // QString symbol_name(symbol_name_buf);

        // qDebug("    st_name: 0x%08X %s", symbol.st_name, file_buf.data() + str_shdrs.sh_offset + symbol.st_name);

        // qDebug("    %d %s",
        //        start_addr,
        //        qPrintable(symbol_name));
    }

    if (is_find_FlashDevice_symbol == false)
    {
        qDebug("[FlashAlgo] not found FlashDevice_symbol");
    }

    if (sizeof(FlashDevice) != FlashDevice_symbol.st_size)
    {
        qDebug("[FlashAlgo] FlashDevice_symbol size mismatch");
    }

    // qDebug("[FlashAlgo] FlashDevice_symbol addr:0x%08X size:0x%X",
    //        FlashDevice_symbol.st_value,
    //        FlashDevice_symbol.st_size);

    memcpy(&flash_device_info, file_buf.data() + PrgCode_section.sh_offset + FlashDevice_symbol.st_value, sizeof(FlashDevice));

    qDebug("[FlashAlgo] FlashDevice:");
    qDebug("[FlashAlgo] Ver: 0x%04X", flash_device_info.Vers);
    qDebug("[FlashAlgo] DevName: %s", flash_device_info.DevName);
    qDebug("[FlashAlgo] DevType: %s", qPrintable(FlashDevice_type_to_str(flash_device_info.DevType)));
    qDebug("[FlashAlgo] DevAdr: 0x%08X", flash_device_info.DevAdr);
    qDebug("[FlashAlgo] szDev: 0x%08X", flash_device_info.szDev);

    qDebug("[FlashAlgo] FuncSymbol:");

    bool is_func_symbol_all_done = true;

    for (uint32_t i = 0; i < FUNC_SYMBOL_COUNT; i++)
    {
        flash_func_code_list[i].clear();

        if (is_find_Init_symbol[i])
        {
            char *pbuf = file_buf.data() + PrgCode_section.sh_offset + flash_func_symbol_list[i].st_value;

            flash_func_code_list[i].append(pbuf, flash_func_symbol_list[i].st_size);
        }

        if ((is_find_Init_symbol[i] == false) && (func_symbol_list[i].must == true))
        {
            is_func_symbol_all_done = false;
        }

        qDebug("[FlashAlgo] %d %16s 0x%08X 0x%08X %10s | %s",
               i,
               func_symbol_list[i].name,
               flash_func_symbol_list[i].st_value,
               flash_func_symbol_list[i].st_size,
               (func_symbol_list[i].must) ? "must" : "optional",
               (is_find_Init_symbol[i] == false) ? "fail" : "ok");
    }

    if (is_func_symbol_all_done == false)
    {
        qDebug("[FlashAlgo] FuncSymbol check fail");
        return -1;
    }

    qDebug("[FlashAlgo] FuncSymbol check all ok");

    return 0;
}

QString FlashAlgo::section_get_name(Elf32_Shdr section)
{
    char *pstr = file_buf.data() + sh_str_section.sh_offset + section.sh_name;

    uint32_t str_name_len = strlen(pstr);
    QByteArray name_buf(pstr, str_name_len);
    QString name(name_buf);

    return name;
}

QString FlashAlgo::symbol_get_name(Elf32_Sym symbol)
{
    char *pstr = file_buf.data() + str_section.sh_offset + symbol.st_name;

    uint32_t str_name_len = strlen(pstr);
    QByteArray name_buf(pstr, str_name_len);
    QString name(name_buf);

    return name;
}

QString FlashAlgo::section_type_to_str(uint32_t section_type)
{
    switch (section_type)
    {
    case SHT_NULL:
        return "SHT_NULL";
    case SHT_PROGBITS:
        return "SHT_PROGBITS";
    case SHT_SYMTAB:
        return "SHT_SYMTAB";
    case SHT_STRTAB:
        return "SHT_STRTAB";
    case SHT_RELA:
        return "SHT_RELA";
    case SHT_HASH:
        return "SHT_HASH";
    case SHT_DYNAMIC:
        return "SHT_DYNAMIC";
    case SHT_NOTE:
        return "SHT_NOTE";
    case SHT_NOBITS:
        return "SHT_NOBITS";
    case SHT_REL:
        return "SHT_REL";
    case SHT_SHLIB:
        return "SHT_SHLIB";
    case SHT_DYNSYM:
        return "SHT_DYNSYM";
    case SHT_INIT_ARRAY:
        return "SHT_INIT_ARRAY";
    case SHT_FINI_ARRAY:
        return "SHT_FINI_ARRAY";
    case SHT_PREINIT_ARRAY:
        return "SHT_PREINIT_ARRAY";
    case SHT_GROUP:
        return "SHT_GROUP";
    case SHT_SYMTAB_SHNDX:
        return "SHT_SYMTAB_SHNDX";
    case SHT_NUM:
        return "SHT_NUM";
    case SHT_LOOS:
        return "SHT_LOOS";
    case SHT_GNU_ATTRIBUTES:
        return "SHT_GNU_ATTRIBUTES";
    case SHT_GNU_HASH:
        return "SHT_GNU_HASH";
    case SHT_GNU_LIBLIST:
        return "SHT_GNU_LIBLIST";
    case SHT_CHECKSUM:
        return "SHT_CHECKSUM";
    case SHT_LOSUNW:
        return "SHT_LOSUNW / SHT_SUNW_move";
    case SHT_SUNW_COMDAT:
        return "SHT_SUNW_COMDAT";
    case SHT_SUNW_syminfo:
        return "SHT_SUNW_syminfo";
    case SHT_GNU_verdef:
        return "SHT_GNU_verdef";
    case SHT_GNU_verneed:
        return "SHT_GNU_verneed";
    case SHT_GNU_versym:
        return "SHT_GNU_versym / SHT_HISUNW / SHT_HIOS";
    case SHT_LOPROC:
        return "SHT_LOPROC";
    case SHT_HIPROC:
        return "SHT_HIPROC";
    case SHT_LOUSER:
        return "SHT_LOUSER";
    case SHT_HIUSER:
        return "SHT_HIUSER";
    }

    return "";
}

QString FlashAlgo::section_flags_to_str(uint32_t section_flags)
{
    QStringList str_list;

    if (section_flags & SHF_WRITE)
        str_list.append("SHF_WRITE");

    if (section_flags & SHF_ALLOC)
        str_list.append("SHF_ALLOC");

    if (section_flags & SHF_EXECINSTR)
        str_list.append("SHF_EXECINSTR");

    if (section_flags & SHF_MERGE)
        str_list.append("SHF_MERGE");

    if (section_flags & SHF_STRINGS)
        str_list.append("SHF_STRINGS");

    if (section_flags & SHF_INFO_LINK)
        str_list.append("SHF_INFO_LINK");

    if (section_flags & SHF_LINK_ORDER)
        str_list.append("SHF_LINK_ORDER");

    if (section_flags & SHF_OS_NONCONFORMING)
        str_list.append("SHF_OS_NONCONFORMING");

    if (section_flags & SHF_GROUP)
        str_list.append("SHF_GROUP");

    if (section_flags & SHF_TLS)
        str_list.append("SHF_TLS");

    if (section_flags & SHF_COMPRESSED)
        str_list.append("SHF_COMPRESSED");

    if (section_flags & SHF_MASKOS)
        str_list.append("SHF_MASKOS");

    if (section_flags & SHF_MASKPROC)
        str_list.append("SHF_MASKPROC");

    if (section_flags & SHF_ORDERED)
        str_list.append("SHF_ORDERED");

    if (section_flags & SHF_EXCLUDE)
        str_list.append("SHF_EXCLUDE");

    QString ret_str = str_list.join(" ");

    if (ret_str.length() == 0)
    {
        ret_str.push_back("SHF_NULL");
    }
    else
    {
        ret_str.push_front("[");
        ret_str.push_back("]");
    }

    return ret_str;
}

QString FlashAlgo::FlashDevice_type_to_str(uint16_t type)
{
    switch (type)
    {
    case UNKNOWN:
        return "UNKNOWN";
    case ONCHIP:
        return "ONCHIP";
    case EXT8BIT:
        return "EXT8BIT";
    case EXT16BIT:
        return "EXT16BIT";
    case EXT32BIT:
        return "EXT32BIT";
    case EXTSPI:
        return "EXTSPI";
    }

    return "";
}

bool FlashAlgo::is_flash_func_impl(flash_func_t f)
{
    if (f >= FLASH_FUNC_MAX)
        return false;

    if (flash_func_code_list[f].count() == 0)
        return false;

    return true;
}