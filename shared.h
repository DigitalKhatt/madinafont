#pragma once

#include "fontplugin.h"
#include "madina.h"

extern "C" DIGITALKHATT_FONT_EXPORT Automedina* font_create(OtLayout* layout, MPFont* font, bool extended);

extern "C" void font_delete(Madina* p_obj) {
	try {
		delete p_obj;
	}
	catch (...) {};
};
