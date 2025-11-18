#include "algorithm.hpp"

#include <random>
#include <tuple>
#include <vector>

double computeEffectivePriority(const WordEntry& entry) {
    const QDateTime now = QDateTime::currentDateTime();

    // ============ 1. 基础权重：用户设定的 priority ============
    double userWeight = std::max(1, entry.priority);

    // ============ 2. 错误率权重（核心） ============
    double errorRateWeight = 1.0;

    // 使用加法平滑（Laplace smoothing）避免小样本偏差
    const double SMOOTHING = 1.0;  // 虚拟“先验”尝试次数
    double smoothedAttempts = entry.totalAttempts + 2 * SMOOTHING;
    double smoothedErrors = entry.totalErrors + SMOOTHING;
    double smoothedErrorRate = smoothedErrors / smoothedAttempts;

    // 将错误率映射为 [1.0, MAX_ERROR_BOOST] 的权重
    const double MAX_ERROR_BOOST = 5.0;  // 最多提升5倍
    errorRateWeight = 1.0 + (MAX_ERROR_BOOST - 1.0) * smoothedErrorRate;

    // ============ 3. 时间衰减因子（越久未练，权重越高） ============
    double timeBoost = 1.0;

    if (entry.totalAttempts > 0 && entry.modifiedAt.isValid()) {
        qint64 hoursSinceReview = entry.modifiedAt.msecsTo(now) / (3600LL * 1000);
        hoursSinceReview = std::max(0LL, hoursSinceReview);

        // 指数增长模型：每24小时，权重 × TIME_DECAY_FACTOR
        const double TIME_DECAY_FACTOR = 1.8;  // 可调：越大，遗忘惩罚越强
        const double TIME_WINDOW_HOURS = 24.0;

        timeBoost = std::pow(TIME_DECAY_FACTOR, hoursSinceReview / TIME_WINDOW_HOURS);

        // 防止数值爆炸（例如几年没练）
        timeBoost = std::min(timeBoost, 10.0);
    } else {
        // 从未练习过的新词：给予适度 boost（但低于高频错词）
        timeBoost = 1.5;
    }

    // ============ 4. 新词加成（可选） ============
    double newWordBonus = 1.0;
    if (entry.totalAttempts == 0) {
        // 刚加入的词，确保有一定曝光
        newWordBonus = 1.2;
    }

    // ============ 5. 综合权重 ============
    double effective = userWeight * errorRateWeight * timeBoost * newWordBonus;

    // 确保权重 ≥ 1（Efraimidis-Spirakis 要求 w > 0）
    effective = std::max(effective, 1.0);

    // 可选：上限防止极端值（对算法稳定性有益）
    effective = std::min(effective, 1000.0);

    return effective;
}

QVector<qsizetype> radomlySelectWords(const QVector<WordEntry>& wordEntries, int count) {
    auto indicesAndK = std::vector<std::tuple<qsizetype, double>>();
    for (qsizetype i = 0; i < wordEntries.size(); ++i) {
        indicesAndK.emplace_back(i, 0.0);
    }

    auto seed = static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    auto gen = std::mt19937(seed);
    auto dis = std::uniform_real_distribution<double>(0.0, 1.0);

    for (auto& [i, k] : indicesAndK) {
        auto u = dis(gen);  // u ∈ [0,1)
        if (u == 0.0)
            u = std::numeric_limits<double>::min();  // 避免 log(0) 或 pow(0, ...)
        k = std::pow(u, 1.0 / computeEffectivePriority(wordEntries[i]));
    }

    std::sort(indicesAndK.begin(), indicesAndK.end(), [](const auto& a, const auto& b) {
        return std::get<1>(a) > std::get<1>(b);
    });

    auto selectedIndices = QVector<qsizetype>();
    for (int i = 0; i < count; ++i) {
        selectedIndices.append(std::get<0>(indicesAndK[i]));
    }
    return selectedIndices;
}

QString randomlySelectDefinition(const WordEntry& wordEntry) {
    QStringList parts = wordEntry.definition.split(",");
    if (parts.isEmpty()) {
        return QString();
    }
    std::random_device rd;
    std::default_random_engine gen(rd());
    std::shuffle(parts.begin(), parts.end(), gen);
    return parts.first().trimmed();
}
