#include "utils.h"
#include <QString>

void hexdump(const uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    uint32_t line_i = 0;

    QString res("");

    res.append("[hexdump] ====================================================\r\n");
    res.append("[hexdump]       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n");

    for (i = 0; i < len; i += 0x10)
    {
        res.append(QString("[hexdump]"));
        res.append(QString(" %1").arg((int)(i / 16), 4, 16, QChar('0')).toUpper());

        for (line_i = 0; line_i < 0x10; line_i++)
        {
            if ((i + line_i) >= len)
            {
                res.append("   ");
                continue;
            }

            res.append(QString(" %1").arg(*(buf + i + line_i), 2, 16, QChar('0')).toUpper());
        }

        res.append("\r\n");
    }

    res.append("[hexdump] ====================================================\r\n");

    qDebug("%s", qPrintable(res));
}
