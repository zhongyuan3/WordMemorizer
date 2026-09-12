#include <QString>
#include <QVector>

#include <iostream>

#include "core/word_selection.h"
#include "data/models.h"

static int g_failures = 0;

#define CHECK(cond)                                                     \
    do {                                                                \
        if (!(cond)) {                                                  \
            std::cerr << "FAILED: " << #cond << " (" << __FILE__ << ":" \
                      << __LINE__ << ")\n";                             \
            ++g_failures;                                               \
        }                                                               \
    } while (0)

int main() {
    QVector<WordEntry> entries;
    for (int i = 0; i < 10; ++i) {
        WordEntry e;
        e.word = QStringLiteral("word%1").arg(i);
        e.definitions = {QStringLiteral("d%1a").arg(i),
                         QStringLiteral("d%1b").arg(i)};
        entries.append(e);
    }

    const auto selected = randomlySelectWords(entries, 5);
    CHECK(selected.size() == 5);
    for (auto idx : selected) {
        CHECK(idx >= 0 && idx < entries.size());
    }

    const auto def = randomlySelectDefinition(entries.first());
    CHECK(def == QStringLiteral("d0a") || def == QStringLiteral("d0b"));

    if (g_failures == 0) {
        std::cout << "All tests passed\n";
        return 0;
    }
    std::cerr << g_failures << " check(s) failed\n";
    return 1;
}