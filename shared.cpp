
#include "shared.h"

__declspec(dllexport) Automedina* font_create(OtLayout* layout, MPFont* font, bool extended) {
    try {
        return new Madina(layout, font, extended);
    }
    catch (...) {
        return nullptr;
    };
}