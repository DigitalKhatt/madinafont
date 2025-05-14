#pragma once

#include "madina.h"

extern "C" __declspec(dllexport) Automedina* font_create(OtLayout* layout, Font* font, bool extended);

extern "C" void font_delete(Madina* p_obj) {
	try {
		delete p_obj;
	}
	catch (...) {};
};