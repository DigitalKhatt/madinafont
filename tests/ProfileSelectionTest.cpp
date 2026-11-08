#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "MPFont.h"
#include "madina.h"

namespace fs = std::filesystem;

static void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

static std::string read(const fs::path& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) throw std::runtime_error("Cannot read " + path.string());
  return {std::istreambuf_iterator<char>{file}, {}};
}

static void initialize(MPFont& font, const fs::path& project) {
  const fs::path resources{MADINA_METAFONT_RESOURCES};
  font.initialize("MPGUI:=1;" + read(resources / "mfplain.mp") +
      read(resources / "mpost.mp") + read(resources / "vmf.mp") + read(project), project);
  const auto sources = read(project.parent_path() / "glyphs.mp");
  for (std::size_t pos = 0; pos < sources.size();) {
    const auto begin = sources.find("beginchar", pos), def = sources.find("defchar", pos);
    const bool isDef = def != std::string::npos && (begin == std::string::npos || def < begin);
    const auto start = isDef ? def : begin;
    if (start == std::string::npos) break;
    const std::string ending = isDef ? "enddefchar;" : "endchar;";
    const auto end = sources.find(ending, start);
    require(end != std::string::npos, "Unterminated glyph source");
    const auto block = sources.substr(start, end + ending.size() - start);
    const auto paren = block.find('('), first = block.find(',', paren), second = block.find(',', first + 1);
    font.registerGlyphSource(block.substr(paren + 1, first - paren - 1), block,
        isDef ? "defchar" : "beginchar", std::stoi(block.substr(first + 1, second - first - 1)));
    font.execute("params[0]:=0;params[1]:=0;params[2]:=0;params[3]:=0;params[4]:=0;" + block);
    pos = end + ending.size();
  }
}


int main(int argc, char** argv) {
  if (argc != 2) return 2;
  const auto project = fs::absolute(argv[1]);
  const auto temporary = fs::temp_directory_path() /
      ("madina-profiles-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
  fs::create_directory(temporary);
  try {
    const auto features = temporary / "profiles.fea";
    std::ofstream(features) << R"(
#if isJustificationProfile
lookup profile.test.justification { feature test; pos space <0 0 1 0>; } profile.test.justification;
#endif;
#if isPortableProfile
lookup profile.test.portable { feature test; pos space <0 0 2 0>; } profile.test.portable;
#endif;
feature test {
#if isJustificationProfile
  lookup profile.test.justification;
#endif;
#if isPortableProfile
  lookup profile.test.portable;
#endif;
} test;
)";
    for (bool extended : {false, true}) {
      for (const std::string selected : {"", "justification", "live", "portable", "legacy"}) {
        MPFont font;
        initialize(font, project);
        if (selected == "legacy") font.execute("string fontProfile;");
        else if (!selected.empty()) font.execute("fontProfile := \"" + selected + "\";");
        OtLayout layout(&font, extended, false);
        const bool portable = selected == "portable";
        require(font.boolVariable("isPortableProfile") == portable &&
                    font.boolVariable("isJustificationProfile") == !portable,
            "Feature conditionals must observe the selected profile");
        require(!font.boolVariable("isForCoreText"),
            "Profile selection must preserve isForCoreText");
        layout.loadLookupFile(features.string());
        require(layout.lookupsIndexByName.contains("profile.test.portable") == portable &&
                    layout.lookupsIndexByName.contains("profile.test.justification") == !portable,
            "Only the selected profile's conditional lookup must be included");
      }
    }
    {
      MPFont font;
      initialize(font, project);
      font.execute("fontProfile := \"portable\"; isForCoreText := true;");
      OtLayout layout(&font, true, false);
      require(font.boolVariable("isForCoreText") && font.boolVariable("isPortableProfile"),
          "isForCoreText and profile selection must remain independent");
    }
    {
      MPFont font;
      initialize(font, project);
      font.execute("fontProfile := \"invalid-profile\";");
      bool rejected = false;
      try { OtLayout layout(&font, true, false); }
      catch (const std::runtime_error& error) {
        rejected = std::string(error.what()).find("font_create failed") != std::string::npos;
      }
      require(rejected, "Unknown profile names must reject plugin construction");
    }
    fs::remove_all(temporary);
    std::cout << "Madina profiles: defaults, explicit selection, feature conditionals, independent settings and errors passed\n";
  } catch (const std::exception& error) {
    fs::remove_all(temporary);
    std::cerr << error.what() << '\n';
    return 1;
  }
}
