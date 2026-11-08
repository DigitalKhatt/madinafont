#pragma once
#include <string>
#include "GlyphVis.h"
#include "Lookup.h"
#include "Subtable.h"
#include "automedina.h"

class Madina : public Automedina {
 public:
  enum class Profile { Justification, Portable };

  Madina(OtLayout* layout, MPFont* font, bool extended);
  Profile fontProfile() const { return profile_; }
  Lookup* getLookup(std::string lookupName) override;
  PairAdjustFunc getPairAdjustFunction(std::string functionName, Subtable* subtable) override;
  CalcAnchor getanchorCalcFunctions(std::string functionName, Subtable* subtable) override;
  void generateSubstEquivGlyphs() override;
  ~Madina() {}

 private:
  Profile profile_ = Profile::Justification;
  bool isForCoreText = false;
  Lookup* defaultmarkposition();
  Lookup* defaultwaqfmarktobase();
  Lookup* forsmalllalef();
  Lookup* forsmallhighwaw();
  Lookup* forhamza();
  Lookup* forheh();
  Lookup* forwaw();
  Lookup* cursivejoinrtl();
  Lookup* pointmarks();
  Lookup* defaultwaqfmarkabovemarkprecise();
  Lookup* defaultdotmarks();
  Lookup* defaultmarkdotmarks();
  Lookup* defaultmkmk();
  Lookup* ayanumbers();
  Lookup* ayanumberskern();
  Lookup* tajweedcolorcpp();
  Lookup* glyphalternates();
  // Justification
  Lookup* shrinkstretchlt(float lt, std::string featureName);
  Lookup* shrinkstretchlt();
  void addchars();
  void generateGlyphs();
};
class Defaultwaqfmarkabovemark : public AnchorCalc {
 public:
  Defaultwaqfmarkabovemark(Automedina& y, MarkBaseSubtable& subtable) : _y(y), _subtable(subtable) {}
  Point operator()(std::string glyphName, std::string className, Point adjust, GlyphParameters parameters) override {
    GlyphVis& curr = _y.glyphs[glyphName];

    int width = 0;
    int height = 0;

    return Point(width, height);
  };

 private:
  Automedina& _y;
  MarkBaseSubtable& _subtable;
};
class Defaultmarkbelowwaqfmark : public AnchorCalc {
 public:
  Defaultmarkbelowwaqfmark(Automedina& y, MarkBaseSubtable& subtable) : _y(y), _subtable(subtable) {}
  Point operator()(std::string glyphName, std::string className, Point adjust, GlyphParameters parameters) override {
    GlyphVis& curr = _y.glyphs[glyphName];

    int width = 0 + adjust.x();
    int height = curr.height + 50 + adjust.y();

    return Point(width, height);
  };

 private:
  Automedina& _y;
  MarkBaseSubtable& _subtable;
};
