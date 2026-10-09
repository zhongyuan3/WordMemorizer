#ifndef WIDGETS_THEME_H
#define WIDGETS_THEME_H

#include <QColor>
#include <QString>

class QWidget;

/**
 * @brief 全局主题（唯一样式来源）
 *
 * 所有设计 token（颜色、圆角、字号）集中在此处定义：
 * - `resources/styles/main.qss` 以 `${token}` 占位符引用颜色/圆角/字号，
 *   由 Theme::styleSheet() 读取后注入，QSS 中不再出现硬编码色值；
 * - 需要按数据变化的项着色，通过模型角色（Qt::BackgroundRole /
 *   Qt::ForegroundRole，值取自本 Theme）实现，无需自定义委托。
 *
 * 这样所有样式都经由同一套数值定义，避免颜色、字号在多个位置
 * 各自维护而导致样式漂移。配色基于 Element UI。
 */
namespace Theme {

/// 颜色 token。命名与 main.qss 中的 ${token} 一一对应。
namespace Color {
// ---- 品牌 / 主色 ----
inline constexpr auto Primary = "#409eff";
inline constexpr auto PrimaryDark = "#337ecc";
inline constexpr auto PrimaryLight = "#66b1ff";
inline constexpr auto PrimaryLightDark = "#5298e6";
inline constexpr auto PrimaryPressed = "#3a8ee6";
inline constexpr auto PrimaryPressedDark = "#2d7fcc";
inline constexpr auto PrimaryFocusBg = "#f0f7ff";
inline constexpr auto PrimarySoft = "#ecf5ff";

// ---- 成功 ----
inline constexpr auto Success = "#67c23a";
inline constexpr auto SuccessDark = "#529b2e";
inline constexpr auto SuccessLight = "#85ce61";
inline constexpr auto SuccessLightDark = "#6bb749";
inline constexpr auto SuccessPressed = "#5daf34";
inline constexpr auto SuccessPressedDark = "#4a8c2a";
inline constexpr auto SuccessSoft = "#f0f9eb";

// ---- 警告 ----
inline constexpr auto Warning = "#e6a23c";

// ---- 危险 ----
inline constexpr auto Danger = "#f56c6c";
inline constexpr auto DangerDark = "#d65454";
inline constexpr auto DangerLight = "#f78989";
inline constexpr auto DangerLightDark = "#e06c6c";
inline constexpr auto DangerPressed = "#e15c5c";
inline constexpr auto DangerPressedDark = "#c04a4a";
inline constexpr auto DangerSoft = "#fef0f0";

// ---- 辅助色（优先级徽章等）----
inline constexpr auto Purple = "#9b59b6";

// ---- 文字 ----
inline constexpr auto TextPrimary = "#303133";
inline constexpr auto TextRegular = "#2c3e50";
inline constexpr auto TextSecondary = "#909399";
inline constexpr auto TextLabel = "#606266";

// ---- 边框 ----
inline constexpr auto BorderBase = "#dcdfe6";
inline constexpr auto BorderLight = "#ebeef5";
inline constexpr auto BorderLighter = "#e4e7ed";

// ---- 背景 ----
inline constexpr auto BgPage = "#f5f7fa";
inline constexpr auto BgWhite = "#ffffff";
inline constexpr auto BgHover = "#f5f7fa";
inline constexpr auto BgSelected = "#ecf5ff";

// ---- 禁用态 ----
inline constexpr auto DisabledBg = "#c0c4cc";
inline constexpr auto DisabledBgDark = "#a8abb2";

// ---- 滚动条 ----
inline constexpr auto ScrollTrack = "#f1f1f1";
inline constexpr auto ScrollHandle = "#c1c1c1";
inline constexpr auto ScrollHandleHover = "#a8a8a8";
}  // namespace Color

/// 尺寸 token。QSS 使用 *Px 字符串；自绘使用整数常量。
namespace Metric {
// 圆角（QSS）
inline constexpr auto RadiusSmPx = "4px";
inline constexpr auto RadiusPx = "6px";
inline constexpr auto RadiusLgPx = "8px";

// 字号（QSS）
inline constexpr auto FontSmPx = "13px";
inline constexpr auto FontBasePx = "14px";
inline constexpr auto FontLgPx = "15px";
inline constexpr auto FontXlPx = "18px";
}  // namespace Metric

/// 按钮样式变体，配合 main.qss 中的 `QPushButton[variant="..."]` 选择器。
namespace Variant {
inline constexpr auto Primary = "primary";
inline constexpr auto Success = "success";
inline constexpr auto Danger = "danger";
inline constexpr auto Warning = "warning";
}  // namespace Variant

/// 由十六进制字符串创建 QColor，供 C++ 侧（如模型角色着色）使用
inline QColor qcolor(const char *hex) {
    return QColor(QString::fromLatin1(hex));
}

/// 读取 main.qss 模板并注入设计 token，返回可直接应用的样式表
QString styleSheet();

/// 为控件设置样式变体（动态属性 variant），配合 QSS 属性选择器
void setVariant(QWidget *widget, const char *variant);

}  // namespace Theme

#endif  // WIDGETS_THEME_H
