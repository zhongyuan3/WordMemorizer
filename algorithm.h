#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <QVector>

#include "worddatabase.h"

QVector<qsizetype> radomlySelectWords(const QVector<WordEntry> &wordEntries,
                                      int count);
QString randomlySelectDefinition(const WordEntry &wordEntry);

#endif
