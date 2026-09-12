#include "core/word_selection.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include <tuple>
#include <vector>

/**
 * 计算单词的有效优先级权重。
 *
 * 综合因素：
 * - 用户设置的优先级 (1-20)
 * - 历史错误率（平滑处理后映射到 [1, 5] 区间）
 * - 遗忘曲线（距上次练习的时间，指数增长，上限 10x）
 * - 新词加成
 */
static double computeEffectivePriority(const WordEntry &entry) {
    const QDateTime now = QDateTime::currentDateTime();

    double userWeight = std::max(1, entry.priority);

    // 错误率权重（Laplace 平滑）
    constexpr double SMOOTHING = 1.0;
    double smoothedAttempts = entry.totalAttempts + 2 * SMOOTHING;
    double smoothedErrors = entry.totalErrors + SMOOTHING;
    double smoothedErrorRate = smoothedErrors / smoothedAttempts;

    constexpr double MAX_ERROR_BOOST = 5.0;
    double errorRateWeight = 1.0 + (MAX_ERROR_BOOST - 1.0) * smoothedErrorRate;

    // 遗忘曲线
    double timeBoost = 1.0;
    if (entry.totalAttempts > 0 && entry.modifiedAt.isValid()) {
        qint64 hoursSinceReview =
            entry.modifiedAt.msecsTo(now) / (3600LL * 1000);
        hoursSinceReview = std::max(0LL, hoursSinceReview);

        constexpr double TIME_DECAY_FACTOR = 1.8;
        constexpr double TIME_WINDOW_HOURS = 24.0;
        timeBoost =
            std::pow(TIME_DECAY_FACTOR, hoursSinceReview / TIME_WINDOW_HOURS);
        timeBoost = std::min(timeBoost, 10.0);
    } else {
        timeBoost = 1.5;  // 从未练习的新词适度加权
    }

    double newWordBonus = (entry.totalAttempts == 0) ? 1.2 : 1.0;

    double effective = userWeight * errorRateWeight * timeBoost * newWordBonus;
    effective = std::max(effective, 1.0);
    effective = std::min(effective, 1000.0);
    return effective;
}

QVector<qsizetype> randomlySelectWords(const QVector<WordEntry> &entries,
                                       int count) {
    if (entries.isEmpty() || count <= 0) return {};

    using IndexedKey = std::tuple<qsizetype, double>;
    std::vector<IndexedKey> indicesAndKeys;
    indicesAndKeys.reserve(entries.size());

    auto seed = static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    for (qsizetype i = 0; i < entries.size(); ++i) {
        double u = dis(gen);
        if (u == 0.0) {
            u = std::numeric_limits<double>::min();
        }
        double key = std::pow(u, 1.0 / computeEffectivePriority(entries[i]));
        indicesAndKeys.emplace_back(i, key);
    }

    // 按 key 降序排列，取前 count 个
    std::sort(indicesAndKeys.begin(), indicesAndKeys.end(),
              [](const IndexedKey &a, const IndexedKey &b) {
                  return std::get<1>(a) > std::get<1>(b);
              });

    QVector<qsizetype> selected;
    selected.reserve(std::min(count, static_cast<int>(indicesAndKeys.size())));
    for (int i = 0; i < count && i < static_cast<int>(indicesAndKeys.size());
         ++i) {
        selected.append(std::get<0>(indicesAndKeys[i]));
    }
    return selected;
}

QString randomlySelectDefinition(const WordEntry &entry) {
    if (entry.definitions.isEmpty()) return QString();

    QStringList defs = entry.definitions;
    std::random_device rd;
    std::default_random_engine gen(rd());
    std::shuffle(defs.begin(), defs.end(), gen);
    return defs.first().trimmed();
}
