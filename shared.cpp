
#include "shared.h"

DIGITALKHATT_FONT_EXPORT Automedina* font_create(OtLayout* layout, MPFont* font, bool extended) {
    try {
        return new Madina(layout, font, extended);
    }
    catch (...) {
        return nullptr;
    };
}
