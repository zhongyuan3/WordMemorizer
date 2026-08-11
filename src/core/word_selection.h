#ifndef WORD_SELECTION_H
#define WORD_SELECTION_H

#include <QVector>

#include "data/models.h"

/**
 * @brief 基于加权随机采样（Efraimidis-Spirakis）选词
 *
 * 权重综合考虑用户优先级、错误率、遗忘曲线等因素。
 *
 * @param entries  全部单词条目
 * @param count    需要选取的数量
 * @return         被选中单词在原数组中的索引列表
 */
QVector<qsizetype> randomlySelectWords(const QVector<WordEntry> &entries,
                                       int count);

/// 从一条单词的释义列表中随机选取一条释义
QString randomlySelectDefinition(const WordEntry &entry);

#endif  // WORD_SELECTION_H
