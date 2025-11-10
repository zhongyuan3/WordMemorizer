#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include <QVector>

#include "worddatabase.hpp"

QVector<qsizetype> radomlySelectWords(const QVector<WordEntry> &wordEntries, int count);
QString randomlySelectDefinition(const WordEntry &wordEntry);

#endif
