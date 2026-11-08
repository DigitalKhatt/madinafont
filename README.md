# Madina version2

Work on this branch uses the live renderer with `isForCoreText=false`.
CoreText compatibility is deferred while the shared font design is developed.

The reference is the live output of commit
`9aeb02c26d8dae4c032820733466c1ee9137e6d6` with:

- `isForCoreText=false`
- Experimental justification / XScale / shrink None
- `qpc_v2_layout`, force disabled

The initial version2 shaping sources match that reference across all 604 pages,
9,046 lines and 850,528 glyph records, including spaces and headings. Glyph
identities, parameters, clusters, advances, offsets, resolved positions, line
widths, font sizes and XScale values are unchanged. This reference covers live
shaping after justification, before collision placement.

## DirectWrite contextual substitutions

The full `context-and-filter-fixes.patch` is applied to the shared source:

- Eight contextual rules include every glyph consumed by their nested ligature
  in the input span: one `miscellaneoussubst` rule, six `tatweels` rules and the
  CGJ/small-high-waw rule.
- `requiredligature` includes `hamzaabove.lamalef` and `smallalef.replacement`
  in its local mark filtering set. This works around DirectWrite losing later
  contextual targets after a mark substitution; it is not a general OpenType
  requirement to re-match replacement marks.
- The original CGJ/small-high-waw rule remains first, followed by a fallback
  for engines that have already removed CGJ.

Against the preceding version2 commit `be1f683`, all 604 pages / 9,046 lines
remain byte-identical in the live Experimental / XScale / shrink None /
`qpc_v2_layout` renderer, with force disabled. Both the whole-glyph layout binary
and its text sidecar are identical. The fresh static standard OTF also preserves
each engine's previous output, and native CoreText and HarfBuzz agree on glyphs,
clusters, drawn positions and line widths across all 9,046 unjustified lines.
GPOS, outlines, cmap, metrics and glyph ordering are unchanged.

This context/filter patch alone removes all 775 DirectWrite glyph-count
mismatches in the unjustified corpus. The separate horizontal GPOS rewrite
is described below. Tracked OTF/WOFF2 exports and WASM binaries are unchanged;
the local macOS font plugin is rebuilt for the horizontal rewrite.

## Yeh hamza seat

`miscellaneoussubst` retains the original `yehshape.isol cgj hamzaabove`
context and adds `yehshape.isol hamzaabove`, using the same existing
`miscellaneoussubst.l2` child to select the dotless hamza seat. The adjacent
fallback also matches in DirectWrite when it excludes CGJ from GSUB matching.

The four post-joining recovery lookups, their three nested helpers, classes
and `rlig` references have been removed to avoid their lookup size and runtime
cost. Other CGJ-related joining differences remain outside this fallback's
scope.

Across all 604 pages / 9,046 lines, removing recovery preserves byte-identical
HarfBuzz and CoreText traces and the full live Experimental / XScale / shrink
None / `qpc_v2_layout` binary and sidecar. The native static export has seven
fewer GSUB lookups and is 22,688 bytes smaller. The adjacent hamza fallback
also eliminates the extra yeh-dot glyphs in the conditional CGJ-removal test;
DirectWrite joining and positioning differences remain.

## Live reh/waw horizontal positioning

`isForCoreText=false` remains the working configuration. Two contextual
SinglePos passes in `features.fea` replace `rehwawcursivecpp` in both live
shaping and standard OTF export:

- `rehwawhorizontal.exit` applies −150 XPlacement and XAdvance to reh/waw,
  with the following initial/isolated glyph in lookahead.
- `rehwawhorizontal.entry` adjusts the following glyph with reh/waw in
  backtrack. Shared SinglePos helpers contain the existing per-pair values.
- Each pass consumes one input glyph and modifies only that glyph. Consecutive
  reh/waw pairs therefore overlap normally; marks are ignored during matching.

SinglePos now accepts `<function name>` in named helpers and inline contextual
rules. `rehwawhorizontalentry.direct` and `.split` compute the former cursive
entry's rounding from the actual glyph width. The direct form truncates the
instance width; the split form truncates the source width and deformation
separately. Live callbacks evaluate the current parameter instance, while
export writes ordinary numeric GPOS values for materialized glyphs. The
font-specific `prepareAnchorResolution()` rewrite and custom serializer are
removed; only the small width functions remain in the font's C++ source.
Function-valued records retain constant export values across SCLX, matching
the former pair policy.

Full 604-page / 9,046-line justified live binaries and sidecars are byte-identical
before and after this rewrite (Experimental / XScale / None / `qpc_v2_layout`,
force disabled). Static HarfBuzz/CoreText traces and variable OTF traces at
SCLX 50, 100 and 200 are also byte-identical within each engine. Every one of
20,460 compiled horizontal pairs, including materialized alternates, has the
same adjustments. All 1,084 overlap/mark controls retain their output.
With the separate isolated-noon edit preserved, DirectWrite has zero glyph,
width or position differences under the existing normalizer-only CGJ-removal
input policy. The comparison baseline includes that same vertical edit, so
it measures only this SinglePos rewrite. All cursive lookups stay RTL. Compared
with the former C++ two-pass exporter, the static OTF is 178,980 bytes smaller
(about 2.3%) and the variable OTF is 169,152 bytes smaller. Outlines, metrics, cmap, GSUB and
glyph ordering are unchanged. A focused regression test covers named/inline
syntax, instance callbacks, all positioning fields, parameters, table splitting
and unknown function errors.

All shared reh/waw vertical pair rules, baseline propagation and distance
passes have been removed. Vertical corrections are now reviewed separately,
starting with the largest differences against the frozen reference.

All focused reh/waw vertical corrections are grouped in `rehwawvertical`,
a contextual lookup in `features.fea`. Its nested SinglePos helper,
`rehwawvertical.offsets`, holds the shared glyph-specific Y adjustments.
`rehwawvertical.contextoffsets` holds the additional context-specific
values for heh, yeh and expanded noon, because these same glyphs need
different offsets after different reh/waw forms. Contexts
ignore marks and apply only to the final glyph listed below:

| Context | Target YPlacement |
| --- | ---: |
| `waw.isol → alef.isol.ii` | +296 |
| `reh.fina.afterseen → behshape.init.beforenoon → noon.fina.afterbeh` | -184 |
| Same context ending in `noon.fina.expa.afterbeh` | -69 |
| `reh.fina → qaf.isol` | -125 |
| `reh.isol → behshape.init.beforeyeh → yehshape.fina.ii` | +124 |
| `reh.fina.afterbehshape → behshape.init.beforereh → reh.fina` | +118 |
| `waw.fina → heh.isol` | -100 |
| `waw.fina → meem.isol` | -83 |
| `reh.fina → heh.isol` | -97 |
| `reh.fina.afterbehshape → alefmaksura.isol` or `yehshape.isol` | +94 |
| `waw.isol → lam.init.beforeyeh → yehshape.fina.ii` | +55 |
| `waw.fina → behshape.init.beforenoon → noon.fina.expa.afterbeh` | -50 |
| `reh.fina.afterbehshape → waw.isol → alef.isol` | +62 to both waw and alef |
| `reh.fina.afterbehshape → behshape.init → lam.fina` | +39 |
| `reh.fina.afterbehshape → lam.isol` | +39 |

The group runs after `terminaljoinbaseline` and before `cursivejoinrtl`.
Moving the terminal glyph lets RTL cursive position its joined section;
isolated bases move before their marks attach. The existing rlig
`alefadjustmentgsub` selects `alef.isol.ii` after isolated waw with
hamzabelow. All adjustments are vertical; horizontal positioning remains
in the two `rehwawhorizontal` passes.

Regrouping the previously separate lookup rules, including moving the
isolated-base corrections to this common stage, leaves all 604 pages and
9,046 lines of live shaping byte-identical. The expanded noon retains the
requested half correction (-69, rounded from half of 137), so page 61 line 5
word 7 still has a 68-unit vertical difference. Page 353 line 8 word 9's
preceding section retains its 36-unit difference; its final beh/noon
section matches. Page 4 line 4 word 3, page 43 line 5 word 8 and page 26
line 7 word 5 match the original reference after their focused corrections.

The basmala correction stays in `basmala.kern.rahim` because it belongs only
to the regular `basm` feature. Its named SinglePos helper gives terminal
`meem.fina.basmala` -174 YPlacement when preceded by
`behshape.medi.basmala`, before RTL cursive. All 111 regular-basmala
occurrences of the fourth word match the reference exactly, including
page 434 line 9 word 4. The first word's meem and Fatiha `bism` variant
are unchanged.

For the third basmala word, `rehwawvertical.offsets` gives the dedicated
`noon.fina.basmala` terminal -86 YPlacement before RTL cursive. This moves
the joined hah/meem/noon section and its marks together in both `basm`
and `bism`, including page 1 line 2 word 3. All 113 occurrences of
ٱلرَّحْمَٰنِ match the original reference exactly. The full-corpus check
restores 904 Y offsets, with no worsened offsets or non-Y changes. All
cursive lookups remain RTL; native HarfBuzz and CoreText still agree on
all 9,046 unjustified lines of the freshly exported standard OTF.

The `waw.fina → heh.isol` correction restores page 103 line 7 word 2
(`قَتَلُوهُ`) exactly, including its justified XScale and stretched lam.
Across the full live corpus it improves 957 Y offsets, with none worsened
and 782 restored exactly. All horizontal fields and justification choices
remain unchanged. The regrouped font also has no native CoreText/HarfBuzz
differences across all 9,046 unjustified standard-OTF lines; this does not
claim full-corpus native CoreText justification parity.

The `waw.fina → meem.isol` correction restores page 501 line 7 word 1
(`يَوْمِ`): the isolated meem and its kasra move down 83 units, retaining
the reference horizontal positions, deformation and justified XScale.
Across the full live corpus this improves 1,349 Y offsets, with none
worsened and 1,213 restored exactly. The earlier waw/meem report also
included the preceding waw/heh correction; together they improved 2,306
Y offsets over its preceding published report.

The `reh.fina → heh.isol` correction restores page 11 line 2 word 10
(`بَقَرَةࣱ`): the isolated heh, its two dots and dammatan move down
97 units without changing horizontal positioning.

The `reh.fina.afterbehshape → alefmaksura.isol` correction restores
page 213 line 6 word 12 (`يُفْتَرَىٰ`): the isolated alef maksura and
its small alef rise 94 units, preserving horizontal positioning and
justified XScale.
All 68 affected Y offsets across the full live corpus now match the
reference exactly.

The `waw.isol → lam.init.beforeyeh → yehshape.fina.ii` correction restores
page 603 line 5 word 9 (`وَلِيَ`) by raising the terminal yeh 55 units.
The `waw.fina → behshape.init.beforenoon → noon.fina.expa.afterbeh`
correction restores page 168 line 15 word 6 (`لَنَكُونَنَّ`) by lowering
the expanded terminal noon 50 units. Both run before RTL cursive so the
joined letters and their marks follow, without changing justification.
These two contexts improve 356 Y offsets across the full corpus, with
none worsened. Including the preceding reh/heh and reh/alef-maksura
corrections, the regenerated comparison covers 1,461 improved offsets
since the waw/meem report, with 1,427 restored exactly and all horizontal
fields unchanged. Native CoreText and HarfBuzz remain identical across
all 9,046 unjustified standard-OTF lines.

The +94 isolated-base rule also restores page 390 line 6 word 1
(`غَيْرِي`) by raising the final isolated yeh; its medial beh stays at
Y=0. In page 35 line 14 word 9 (`تَبَرُّوا۟`), the waw and following alef
both receive +62 in their shared context after `reh.fina.afterbehshape`.
The small rounded zero follows the alef through existing mark attachment.
Both adjustments use `rehwawvertical.offsets`, with no new helper.
Across all 604 pages they restore all 289 affected Y offsets exactly
to the reference, with horizontal positioning and justification unchanged.
Native CoreText/HarfBuzz equality is retained on all 9,046 unjustified
standard-OTF lines.

The shared +39 lam adjustment restores page 15 line 7 word 3
(`لِّجِبْرِيلَ`), by moving the terminal lam and its joined yeh before
RTL cursive, and page 16 line 14 word 2 (`يُنَزَّلَ`), by moving the
isolated lam before its fatha attaches. Both contexts follow
`reh.fina.afterbehshape` and use the existing `offsets` helper.
Across all 604 pages, all 133 affected Y offsets now match the reference
exactly, with horizontal positioning and justification unchanged.
Native CoreText/HarfBuzz equality is retained on all 9,046 unjustified
standard-OTF lines.

The collision-word follow-up adds shared contexts for the joined sections
in أوبي، ترين، ريبهم، طوبى، عربي، مفتريات، مقترنين، مرجعهم، وبصدهم،
وسنزيد، ولنرسلن، ويربي، يفترينه، افترينا and يكنزون. All 20 matching
corpus occurrences, including vowel variants, match the original reference.
Terminal corrections preserve the actual stretched glyphs and horizontal
positions. The two sections in `وَلَنُرْسِلَنَّ` receive separate terminal
shifts of +8 and +47; neither correction depends on page or word indices.

The same yeh terminal requires +80 after isolated waw, +34 after final
waw, and +33 after final reh. These values have named nested helpers;
`noonafterreh` provides the +39 value needed by ordinary or expanded noon
in مقترنين. The other new contexts reuse `offsets` and `contextoffsets`.
All helpers remain inside the single `rehwawvertical` contextual group.
Across the full corpus the collision-word rules improve 1,746 Y offsets,
with none worsened and 1,682 restored exactly. Every horizontal field,
glyph deformation parameter, line position, font size and XScale is
unchanged.

Ordinary cursive joins remain RTL. The obsolete CoreText reh/waw wrapper,
cursive replay, terminal-attachment helpers and old reh/waw parameter object
remain removed. No shaping-engine changes or buffer adjustments are used.
Production OTF/WOFF2, native plugins and WASM exports are unchanged in this
follow-up. CoreText compatibility remains deferred.

## Mark placement

The existing parent-before-child placement order remains:

- `positionmarkbases` runs before `defaultmarkposition`, keeping the
  existing alef base adjustment in the `mark` feature so justification
  continues to measure widths as before.
- `lowmarkafterwawandreh.mark` runs at the end of `mark`, after
  `defaultmarkposition` and before `defaultmkmk`. It contains the four
  independent lower-vowel contexts using `lowmarks.l2`, `.l4`, `.l5`,
  and `.l6`. The other independent lower-vowel rules retain their timing.
- `positionmarkdots` completes the existing lower/upper dot corrections
  before `defaultmarkdotmarks` attaches dependent marks.
- `positionmarkparents` runs after `defaultmarkdotmarks` and before
  `defaultmkmk`, completing the existing tah, shadda, small-alef, and
  upper-hamza corrections before their dependent marks attach.
- `caltmkmk` resolves the hamzabelow/vowel chain in one contextual rule:
  `hamzaabovebelow.l1` places hamzabelow, the called `hamzabelow.rehwaw`
  helper applies its existing contextual adjustment, and
  `hamzaabovebelow.l2` attaches kasra/kasratan. A fallback calls only the
  contextual adjustment when that custom vowel context does not match.
  The former global `positionhamzabelowparents` pass is removed. Original
  anchor functions, parameter keys, values, and filtering set are retained.
- `positionwaqfparents` completes existing vowel corrections after
  `defaultmkmk` and before contextual waqf attachment in `caltmkmk`.
- `positionsmalllowmeemparents` completes the existing waw/reh lower-dot
  and kasra corrections after `caltmkmk`, including the special waw-hah
  context. `defaultmkmksmalllowmeem` then attaches
  smalllowmeem to the final kasra position, once. The later
  `lowmarkafterwawandreh` pass retains smalllowmeem's own adjustment.
  Its attachment definition moved from the C++ `defaultmkmk` subtable to
  `features.fea`; the original anchor functions and parameter values remain
  unchanged. Its parameter entry now lives under
  `defaultmkmksmalllowmeem.subtable1.smalllowmeem`.

Child corrections remain in the later passes. The existing helpers in
`adjustmkmk` and `lowmarkafterwawandreh` use descriptive names and are
declared immediately before their first contextual use. The lam top-mark
helper is shared by two stages and is declared outside both. Seventeen
matching `parameters.json` root keys have been renamed; their stored values
are unchanged. These explicit helpers no longer use the reserved `_auto`
naming pattern. Five fully shadowed combined context rules have been
removed; the parent and
child stages reference each helper directly. The alef/lam shadda guard and
its broader top-mark fallback remain distinct because both are needed.
Each adjustment runs once.

Helpers used by a single lookup are nested immediately before their first
use in that lookup. This includes the four lower-vowel helpers in
`lowmarkafterwawandreh.mark`, the six lower-dot/vowel helpers in
`positionsmalllowmeemparents`, and the exclusive parent helpers.
The same locality rule applies to the small-alef and tajweed-color helpers.
Shared helpers keep their existing declarations.

The identical, unparameterized kaf/lam top-mark correction is declared once
as `adjustmkmk.kaflam.topmarks`. The narrow kaf/damma context in
`adjustinterlineoverlap` was removed because its general kaf/damma rule
already invokes the same helper with the same inputs. Lookalike dot and
lower-vowel attachments remain separate because their stored anchor
adjustments differ. No parameter keys or values changed in this cleanup.

The other twelve previously split parent/child context groups retain their
placement stages. Combining each at its earliest or latest stage changed
HarfBuzz or native CoreText output in the corpus tests. The hamzabelow
sequence is consolidated because its existing custom vowel attachment can
run immediately after both parent operations, without replaying attachment.

The mark-placement cleanup was validated at the initial version2 checkpoint
against the reference above. The current reh/waw experiment intentionally
changes vertical positions while preserving the horizontal layout.

The reh/waw isolated-noon context covers both `noon.isol` and
`noon.isol.expa`. Both waw and noon receive +62, including when
justification is disabled. This fixes the ordinary-noon collision in
يَكْنِزُونَ and restores its original live placement; the already-corrected
expanded-noon result is unchanged. Horizontal positioning is unchanged.

The same ordinary/expanded coverage applies to the final noon in
مُقْتَرِنِينَ (+39 via `noonafterreh`) and وَلَنُرْسِلَنَّ (+47).
The original glyph contexts and values are retained; only the terminal
classes include both forms. All 15 requested words now match the original
live reference without justification as well as in their tested justified
corpus occurrences.

Additional unstretched contexts cover تنزيلًا, فيرسل, ليريه, وليربط,
البروج, تبروهم, لترونها and لتزول. Existing +39/+62 terminal values
are shared where possible. The waw/lam/reh → beh/tah context adds only
+10 to tah because the initial already rises by 37 through its RTL join.
The reh/waw → beh/heh/alef context uses +62 for waw and +37 for terminal
alef because the initial already rises by 25. These are glyph-context
rules in the same group, with no page or word indices; they also apply
when justification selects those contexts. Their full-corpus effects are
reported separately from exact natural-word matches.

The isolated-waw → behshape.init.beforenoon → noon.fina.afterbeh
context lowers its ordinary terminal noon by 138 units before RTL cursive.
This restores page 173 line 9 word 6 (ٱلْغَاوِينَ) and its matching glyph
contexts, including their attached marks. The existing expanded-noon
rule is unchanged. Horizontal positioning and glyph deformation are
unchanged; all 24 requested natural words still match the original live
reference.

Ordinary final noon after waw.fina receives −73, and after reh.isol
receives −69, in the same behshape.init.beforenoon context. The terminal
corrections run before RTL cursive, restoring page 137 line 6 word 9
(لَا͏ٔ͏َكُونَنَّ) and page 33 line 6 word 1 (وَمُنذِرِينَ). Existing expanded-noon
corrections and horizontal layout are unchanged. All 26 requested natural
words and the previously checked justified cases retain exact reference
matches.

The waw.isol → lam.init.beforeyeh context now includes expanded
yehshape.fina.ii.expa in its existing +55 terminal correction. The matching
fehshape.init.beforeyeh context gives ordinary and expanded yeh +46.
These cover ا͏ٔ͏ُو۟لِي and وَفِي with expanded yeh, while retaining the
ordinary forms. Existing RTL cursive and mark attachments propagate the
terminal shift; horizontal positions and deformation parameters are
unchanged. All 28 requested natural words retain exact reference matches.

The final-reh → behshape.init.beforenoon context now covers expanded
noon with −64, restoring page 10 line 15 word 8 (ٱلنَّٰظِرِينَ). Its ordinary
noon correction remains −87 in the same named helper. The full-corpus
comparison reports the small residual differences in the other matching
expanded contexts; all changed positions improve. All 29 natural-word
tests continue to match the original reference exactly.
The former dot-only correction for لِيُرِيَهُمَا and سَنُرِيهِمْ is removed,
including its dedicated MarkBase lookup and parameter entry. The
`rehwawvertical.subwordafterbehshapereh` SinglePos helper gives the terminal
glyph +36 YPlacement for subwords beginning with reh.fina.afterbehshape →
behshape.init, followed by zero to eight medials and a final (2–10 base
glyphs total). It is a fallback after the existing reviewed contexts.
RTL cursive carries the terminal shift through the connected subword;
normal mark attachment then positions its dots and vowels. A preceding
medial-heh context uses +14 instead, because that join already raises the
initial by 25 units. Thus لِيُرِيَهُمَا and سَنُرِيهِمْ retain the exact
original justified placements, including their dots.

Direct behshape.init → alef.fina sections after reh.fina.afterbehshape
use +39; direct behshape.init → kaf.fina sections use +38, since the kaf
join contributes one unit to the initial's height. These focused rules
make ٱلْكِبْرِيَا͏ٓءُ and لِنُرِيَكَ match the original reference exactly.
The +36 common fallback leaves سَيُرِيكُمْ three units below the reference.
The fixed common value cannot reconstruct every chain's anchor-dependent
baseline: compared with the +14 fallback (after the kaf correction), it
improves 78 glyph offsets but worsens 96 across 27 words on 26 lines.
The kaf correction itself restores 27 offsets in six words exactly.
All non-Y glyph and line fields are unchanged. The 37 natural-word checks
include 36 exact reference matches and the same three-unit residual in
سَيُرِيكُمْ. All cursive lookups remain RTL. Native HarfBuzz/CoreText
output remains identical on all 9,046 unjustified lines.

Three further reh/noon/heh contexts restore the original live placements.
Expanded final noon after reh.isol receives −46 beside the existing
ordinary-noon −69. Isolated heh after reh.fina.afterbehshape receives +39,
and isolated noon receives the same +39 when damma follows that reh.
The damma context leaves the distinct fatha placement unchanged. The
existing +39 helper is shared as rehwawvertical.afterbehshapereh; no
parameter values or horizontal positioning change. All three requested
justified words and all 32 natural-word checks match the original live
reference. The full-corpus comparison restores 208 offsets across 63
words, with none worsened.
