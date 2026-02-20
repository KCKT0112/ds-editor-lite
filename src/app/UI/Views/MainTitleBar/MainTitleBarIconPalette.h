#ifndef MAINTITLEBARICONPALETTE_H
#define MAINTITLEBARICONPALETTE_H

#include "UI/Utils/IconUtils.h"

#include <QColor>

namespace MainTitleBarIconPalette {

    inline QColor normalColor() {
        return QColor(240, 240, 240, 255);
    }

    inline QColor disabledColor() {
        return QColor(240, 240, 240, 102);
    }

    inline IconUtils::SvgIconColorPalette actionPalette() {
        IconUtils::SvgIconColorPalette palette;
        palette.normal = normalColor();
        palette.disabled = disabledColor();
        return palette;
    }

    inline IconUtils::SvgIconToggleColorPalette toggledPalette(const QColor &checkedColor) {
        IconUtils::SvgIconToggleColorPalette palette;
        palette.off = actionPalette();

        QColor checkedDisabledColor = checkedColor;
        checkedDisabledColor.setAlpha(disabledColor().alpha());

        palette.on.normal = checkedColor;
        palette.on.disabled = checkedDisabledColor;
        return palette;
    }

}

#endif // MAINTITLEBARICONPALETTE_H
