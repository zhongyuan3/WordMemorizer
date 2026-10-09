#include "widgets/theme.h"

#include <QFile>
#include <QHash>
#include <QStyle>
#include <QWidget>

namespace {

/// QSS 模板占位符 ${key} 到实际数值的映射。
/// 新增样式 token 时，只需在 Theme::Color / Theme::Metric 与下表各加一处。
const QHash<QString, QString> &tokenMap() {
    static const QHash<QString, QString> tokens = {
        // ---- 主色 ----
        {"primary", Theme::Color::Primary},
        {"primaryDark", Theme::Color::PrimaryDark},
        {"primaryLight", Theme::Color::PrimaryLight},
        {"primaryLightDark", Theme::Color::PrimaryLightDark},
        {"primaryPressed", Theme::Color::PrimaryPressed},
        {"primaryPressedDark", Theme::Color::PrimaryPressedDark},
        {"primaryFocusBg", Theme::Color::PrimaryFocusBg},
        {"primarySoft", Theme::Color::PrimarySoft},
        // ---- 成功 ----
        {"success", Theme::Color::Success},
        {"successDark", Theme::Color::SuccessDark},
        {"successLight", Theme::Color::SuccessLight},
        {"successLightDark", Theme::Color::SuccessLightDark},
        {"successPressed", Theme::Color::SuccessPressed},
        {"successPressedDark", Theme::Color::SuccessPressedDark},
        {"successSoft", Theme::Color::SuccessSoft},
        // ---- 警告 ----
        {"warning", Theme::Color::Warning},
        // ---- 危险 ----
        {"danger", Theme::Color::Danger},
        {"dangerDark", Theme::Color::DangerDark},
        {"dangerLight", Theme::Color::DangerLight},
        {"dangerLightDark", Theme::Color::DangerLightDark},
        {"dangerPressed", Theme::Color::DangerPressed},
        {"dangerPressedDark", Theme::Color::DangerPressedDark},
        {"dangerSoft", Theme::Color::DangerSoft},
        // ---- 辅助色 ----
        {"purple", Theme::Color::Purple},
        // ---- 文字 ----
        {"textPrimary", Theme::Color::TextPrimary},
        {"textRegular", Theme::Color::TextRegular},
        {"textSecondary", Theme::Color::TextSecondary},
        {"textLabel", Theme::Color::TextLabel},
        // ---- 边框 ----
        {"borderBase", Theme::Color::BorderBase},
        {"borderLight", Theme::Color::BorderLight},
        {"borderLighter", Theme::Color::BorderLighter},
        // ---- 背景 ----
        {"bgPage", Theme::Color::BgPage},
        {"bgWhite", Theme::Color::BgWhite},
        {"bgHover", Theme::Color::BgHover},
        {"bgSelected", Theme::Color::BgSelected},
        // ---- 禁用态 ----
        {"disabledBg", Theme::Color::DisabledBg},
        {"disabledBgDark", Theme::Color::DisabledBgDark},
        // ---- 滚动条 ----
        {"scrollTrack", Theme::Color::ScrollTrack},
        {"scrollHandle", Theme::Color::ScrollHandle},
        {"scrollHandleHover", Theme::Color::ScrollHandleHover},
        // ---- 尺寸 ----
        {"radiusSm", Theme::Metric::RadiusSmPx},
        {"radius", Theme::Metric::RadiusPx},
        {"radiusLg", Theme::Metric::RadiusLgPx},
        {"fontSm", Theme::Metric::FontSmPx},
        {"fontBase", Theme::Metric::FontBasePx},
        {"fontLg", Theme::Metric::FontLgPx},
        {"fontXl", Theme::Metric::FontXlPx},
    };
    return tokens;
}

QString applyTokens(QString text) {
    const QHash<QString, QString> &tokens = tokenMap();
    for (auto it = tokens.constBegin(); it != tokens.constEnd(); ++it) {
        text.replace(QStringLiteral("${") + it.key() + QStringLiteral("}"),
                     it.value());
    }
    return text;
}

}  // namespace

QString Theme::styleSheet() {
    QFile file(QStringLiteral(":/resources/styles/main.qss"));
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return QString();
    }
    return applyTokens(QString::fromUtf8(file.readAll()));
}

void Theme::setVariant(QWidget *widget, const char *variant) {
    if (!widget) return;
    widget->setProperty("variant", QString::fromLatin1(variant));
    // 动态属性变化后需重新 polish，样式选择器才会立即生效
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
}
