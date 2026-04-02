#pragma once
#include <QString>
#include <QColor>

namespace ColorUtils {
    // Base dark palette (matches other LGA_ projects)
    inline constexpr auto BG_DARK      = "#161616";
    inline constexpr auto BG_PANEL     = "#1d1d1d";
    inline constexpr auto BG_HOVER     = "#252525";
    inline constexpr auto TEXT_PRIMARY = "#b2b2b2";
    inline constexpr auto TEXT_DIM     = "#666666";
    inline constexpr auto ACCENT       = "#774dcb";
    inline constexpr auto ACCENT_HOVER = "#8e6ddf";
    inline constexpr auto BORDER       = "#333333";
    inline constexpr auto BORDER_FOCUS = "#555555";
    inline constexpr auto SUCCESS      = "#4caf7d";
    inline constexpr auto ERROR_COLOR  = "#cf6679";
    inline constexpr auto WARNING      = "#e8a838";

    QString getStyleSheet();
}
