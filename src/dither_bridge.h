// dither_bridge.h — img2spec adapter over vendored libdither (src/libdither/).
// Libdither does the dithering math; this file only:
//  - registries (UI index -> libdither getter) for all algorithms,
//  - Device palette -> CachedPalette (all 10 comparison modes),
//  - gBitmapProcFloat (BGR float layout!) <-> ColorImage / DitherImage,
//  - deterministic pre-jitter for the COLOR path (upstream color ditherers
//    have no sigma parameter; mono ditherers use libdither's own sigma).
// Memory: every libdither object created here is freed by the caller in the
// same process() call, except CachedPalette which modifiers cache across
// frames (fingerprint: device name + palette size + compare mode).

#ifndef DITHER_BRIDGE_H
#define DITHER_BRIDGE_H

#include <string.h>
#include <math.h>
#include "libdither/libdither.h"

// All comparison modes; indices match enum ColorComparisonMode.
static const char *LD_COMPARE_COMBO =
	"Luminance\0"
	"sRGB\0"
	"Linear\0"
	"HSV\0"
	"LAB76\0"
	"LAB94\0"
	"LAB2000\0"
	"sRGB CCIR\0"
	"Linear CCIR\0"
	"Tetrapal\0";
#define LD_COMPARE_COUNT 10

// ---------------- error diffusion: 19 kernels ----------------
// Indices 0..6 keep the legacy img2spec order so old .isw files map 1:1.
typedef ErrorDiffusionMatrix *(*LDErrDiffFn)();
struct LDErrDiffEntry { const char *name; LDErrDiffFn fn; };
static LDErrDiffEntry LD_ERRDIFF[] =
{
	{ "Floyd-Steinberg", get_floyd_steinberg_matrix },
	{ "Jarvis-Judice-Ninke", get_jarvis_judice_ninke_matrix },
	{ "Stucki", get_stucki_matrix },
	{ "Burkes", get_burkes_matrix },
	{ "Sierra3", get_sierra_3_matrix },
	{ "Sierra2", get_sierra_2row_matrix },
	{ "Sierra Lite", get_sierra_lite_matrix },
	{ "Diagonal", get_diagonal_matrix },
	{ "ShiauFan 1", get_shiaufan1_matrix },
	{ "ShiauFan 2", get_shiaufan2_matrix },
	{ "ShiauFan 3", get_shiaufan3_matrix },
	{ "Diffusion 1D", get_diffusion_1d_matrix },
	{ "Diffusion 2D", get_diffusion_2d_matrix },
	{ "Fake Floyd-Steinberg", get_fake_floyd_steinberg_matrix },
	{ "Atkinson", get_atkinson_matrix },
	{ "Steve Pigeon", get_steve_pigeon_matrix },
	{ "Robert Kist", get_robert_kist_matrix },
	{ "Stevenson-Arce", get_stevenson_arce_matrix },
	{ "Xot", get_xot_matrix },
};
#define LD_ERRDIFF_COUNT 19

static const char *LD_ERRDIFF_COMBO =
	"Floyd-Steinberg\0"
	"Jarvis-Judice-Ninke\0"
	"Stucki\0"
	"Burkes\0"
	"Sierra3\0"
	"Sierra2\0"
	"Sierra Lite\0"
	"Diagonal\0"
	"ShiauFan 1\0"
	"ShiauFan 2\0"
	"ShiauFan 3\0"
	"Diffusion 1D\0"
	"Diffusion 2D\0"
	"Fake Floyd-Steinberg\0"
	"Atkinson\0"
	"Steve Pigeon\0"
	"Robert Kist\0"
	"Stevenson-Arce\0"
	"Xot\0";

// ---------------- ordered: 40 fixed + 3 parametric ----------------
typedef OrderedDitherMatrix *(*LDOrderedFn)();
struct LDOrderedEntry { const char *name; LDOrderedFn fn; };
static LDOrderedEntry LD_ORDERED[] =
{
	{ "Bayer 2x2", get_bayer2x2_matrix },
	{ "Bayer 3x3", get_bayer3x3_matrix },
	{ "Bayer 4x4", get_bayer4x4_matrix },
	{ "Bayer 8x8", get_bayer8x8_matrix },
	{ "Bayer 16x16", get_bayer16x16_matrix },
	{ "Bayer 32x32", get_bayer32x32_matrix },
	{ "Blue Noise 128x128", get_blue_noise_128x128 },
	{ "Dispersed Dots 1", get_dispersed_dots_1_matrix },
	{ "Dispersed Dots 2", get_dispersed_dots_2_matrix },
	{ "Void Dispersed Dots", get_ulichney_void_dispersed_dots_matrix },
	{ "Non-Rectangular 1", get_non_rectangular_1_matrix },
	{ "Non-Rectangular 2", get_non_rectangular_2_matrix },
	{ "Non-Rectangular 3", get_non_rectangular_3_matrix },
	{ "Non-Rectangular 4", get_non_rectangular_4_matrix },
	{ "Ulichney Bayer 5x5", get_ulichney_bayer_5_matrix },
	{ "Ulichney", get_ulichney_matrix },
	{ "Clustered Dot 1", get_bayer_clustered_dot_1_matrix },
	{ "Clustered Dot 2", get_bayer_clustered_dot_2_matrix },
	{ "Clustered Dot 3", get_bayer_clustered_dot_3_matrix },
	{ "Clustered Dot 4", get_bayer_clustered_dot_4_matrix },
	{ "Clustered Dot 5", get_bayer_clustered_dot_5_matrix },
	{ "Clustered Dot 6", get_bayer_clustered_dot_6_matrix },
	{ "Clustered Dot 7", get_bayer_clustered_dot_7_matrix },
	{ "Clustered Dot 8", get_bayer_clustered_dot_8_matrix },
	{ "Clustered Dot 9", get_bayer_clustered_dot_9_matrix },
	{ "Clustered Dot 10", get_bayer_clustered_dot_10_matrix },
	{ "Clustered Dot 11", get_bayer_clustered_dot_11_matrix },
	{ "Central White Point", get_central_white_point_matrix },
	{ "Balanced Center Point", get_balanced_centered_point_matrix },
	{ "Diagonal", get_diagonal_ordered_matrix_matrix },
	{ "Ulichney Clustered Dot", get_ulichney_clustered_dot_matrix },
	{ "Magic Circle 5x5", get_magic5x5_circle_matrix },
	{ "Magic Circle 6x6", get_magic6x6_circle_matrix },
	{ "Magic Circle 7x7", get_magic7x7_circle_matrix },
	{ "Magic 45deg 4x4", get_magic4x4_45_matrix },
	{ "Magic 45deg 6x6", get_magic6x6_45_matrix },
	{ "Magic 45deg 8x8", get_magic8x8_45_matrix },
	{ "Magic 4x4", get_magic4x4_matrix },
	{ "Magic 6x6", get_magic6x6_matrix },
	{ "Magic 8x8", get_magic8x8_matrix },
	{ "Variable 2x2 (step)", 0 },
	{ "Variable 4x4 (step)", 0 },
	{ "Interleaved Gradient", 0 },
};
#define LD_ORDERED_FIXED 40
#define LD_ORDERED_COUNT 43

static const char *LD_ORDERED_COMBO =
	"Bayer 2x2\0" "Bayer 3x3\0" "Bayer 4x4\0" "Bayer 8x8\0" "Bayer 16x16\0" "Bayer 32x32\0"
	"Blue Noise 128x128\0"
	"Dispersed Dots 1\0" "Dispersed Dots 2\0" "Void Dispersed Dots\0"
	"Non-Rectangular 1\0" "Non-Rectangular 2\0" "Non-Rectangular 3\0" "Non-Rectangular 4\0"
	"Ulichney Bayer 5x5\0" "Ulichney\0"
	"Clustered Dot 1\0" "Clustered Dot 2\0" "Clustered Dot 3\0" "Clustered Dot 4\0"
	"Clustered Dot 5\0" "Clustered Dot 6\0" "Clustered Dot 7\0" "Clustered Dot 8\0"
	"Clustered Dot 9\0" "Clustered Dot 10\0" "Clustered Dot 11\0"
	"Central White Point\0" "Balanced Center Point\0" "Diagonal\0" "Ulichney Clustered Dot\0"
	"Magic Circle 5x5\0" "Magic Circle 6x6\0" "Magic Circle 7x7\0"
	"Magic 45deg 4x4\0" "Magic 45deg 6x6\0" "Magic 45deg 8x8\0"
	"Magic 4x4\0" "Magic 6x6\0" "Magic 8x8\0"
	"Variable 2x2 (step)\0" "Variable 4x4 (step)\0" "Interleaved Gradient\0";

// Parametric ordered factory. Caller frees with OrderedDitherMatrix_free().
static OrderedDitherMatrix *LD_GetOrdered(int idx, int varStep, int gradSize, double ga, double gb, double gc)
{
	if (idx < 0) idx = 0;
	if (idx >= LD_ORDERED_COUNT) idx = 0;
	if (idx < LD_ORDERED_FIXED)
		return LD_ORDERED[idx].fn();
	if (idx == LD_ORDERED_FIXED)
		return get_variable_2x2_matrix(varStep);
	if (idx == LD_ORDERED_FIXED + 1)
		return get_variable_4x4_matrix(varStep);
	return get_interleaved_gradient_noise(gradSize, ga, gb, gc);
}

// Cyclically shifted copy (for X/Y offsets). Returns 0 when no shift.
// Caller frees the result with OrderedDitherMatrix_free().
static OrderedDitherMatrix *LD_ShiftOrdered(OrderedDitherMatrix *m, int xofs, int yofs)
{
	int w = m->width, h = m->height;
	xofs %= w; if (xofs < 0) xofs += w;
	yofs %= h; if (yofs < 0) yofs += h;
	if (xofs == 0 && yofs == 0)
		return 0;
	int *buf = new int[w * h];
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
			buf[y * w + x] = m->buffer[((y + yofs) % h) * w + ((x + xofs) % w)];
	OrderedDitherMatrix *out = OrderedDitherMatrix_new(w, h, m->divisor, buf);
	delete[] buf;
	return out;
}

// ---------------- mono families ----------------
typedef RiemersmaCurve *(*LDCurveFn)();
static LDCurveFn LD_CURVES[] =
{
	get_hilbert_curve, get_hilbert_mod_curve, get_peano_curve,
	get_fass0_curve, get_fass1_curve, get_fass2_curve,
	get_gosper_curve, get_fass_spiral_curve,
};
#define LD_CURVE_COUNT 8
static const char *LD_CURVE_COMBO =
	"Hilbert 1\0" "Hilbert 2 (mod)\0" "Peano\0"
	"Fass 0\0" "Fass 1\0" "Fass 2\0" "Gosper\0" "Fass Spiral\0";

typedef TilePattern *(*LDPatternFn)();
static LDPatternFn LD_PATTERNS[] =
{
	get_2x2_pattern, get_3x3_v1_pattern, get_3x3_v2_pattern,
	get_3x3_v3_pattern, get_4x4_pattern, get_5x2_pattern,
};
#define LD_PATTERN_COUNT 6
static const char *LD_PATTERN_COMBO =
	"2x2\0" "3x3 v1\0" "3x3 v2\0" "3x3 v3\0" "4x4\0" "5x2\0";

typedef DotDiffusionMatrix *(*LDDotDiffFn)();
static LDDotDiffFn LD_DOTDIFFS[] =
{
	get_default_diffusion_matrix, get_guoliu8_diffusion_matrix, get_guoliu16_diffusion_matrix,
};
#define LD_DOTDIFF_COUNT 3
static const char *LD_DOTDIFF_COMBO = "Default\0GuoLiu 8x8\0GuoLiu 16x16\0";

typedef DotClassMatrix *(*LDDotClassFn)();
static LDDotClassFn LD_DOTCLASS[] =
{
	get_mini_knuth_class_matrix, get_knuth_class_matrix, get_optimized_knuth_class_matrix,
	get_mese_8x8_class_matrix, get_mese_16x16_class_matrix,
	get_guoliu_8x8_class_matrix, get_guoliu_16x16_class_matrix,
	get_spiral_class_matrix, get_spiral_inverted_class_matrix,
};
#define LD_DOTCLASS_COUNT 9
static const char *LD_DOTCLASS_COMBO =
	"Mini Knuth\0" "Knuth\0" "Optimized Knuth\0"
	"Mese 8x8\0" "Mese 16x16\0" "GuoLiu 8x8\0" "GuoLiu 16x16\0"
	"Spiral\0" "Spiral Inverted\0";

typedef DotLippensCoefficients *(*LDLippensFn)();
static LDLippensFn LD_LIPPENS[] =
{
	get_dotlippens_coefficients1, get_dotlippens_coefficients2, get_dotlippens_coefficients3,
};
#define LD_LIPPENS_COUNT 3
static const char *LD_LIPPENS_COMBO = "Coefficients 1\0Coefficients 2\0Coefficients 3\0";

// ---------------- palette cache ----------------
// Rebuild only when device palette or compare mode changes; per-frame
// lookup hash is dropped by the caller (CachedPalette_free_cache) to bound
// memory on long videos.
static void LD_FreePalette(CachedPalette *&pal)
{
	if (pal) { CachedPalette_free(pal); pal = 0; }
}

static void LD_EnsurePalette(CachedPalette *&pal, char *devName, int devNameCap,
	int &savedCount, int &savedMode, Device *dev, int mode)
{
	int count = dev->palette_count();
	bool same = (pal != 0 && savedMode == mode && savedCount == count &&
		strncmp(devName, dev->getname(), (size_t)devNameCap) == 0);
	if (same)
		return;
	LD_FreePalette(pal);
	strncpy(devName, dev->getname(), (size_t)(devNameCap - 1));
	devName[devNameCap - 1] = 0;
	savedCount = count;
	savedMode = mode;
	BytePalette *bp = BytePalette_new((size_t)count);
	for (int i = 0; i < count; i++)
	{
		int c = dev->palette_entry(i);
		ByteColor bc;
		bc.r = (uint8_t)((c >> 16) & 0xff);
		bc.g = (uint8_t)((c >> 8) & 0xff);
		bc.b = (uint8_t)((c >> 0) & 0xff);
		bc.a = 255;
		BytePalette_set(bp, (size_t)i, &bc);
	}
	pal = CachedPalette_new();
	CachedPalette_from_BytePalette(pal, bp);
	BytePalette_free(bp);
	CachedPalette_update_cache(pal, (enum ColorComparisonMode)mode, 0);
	CachedPalette_set_shift(pal, 1, 1, 1);
}

// ---------------- float <-> libdither images ----------------
// NOTE: img2spec float layout is [+0]=B, [+1]=G, [+2]=R (see quantize/ordered).
static int LD_FloatToByte(float v)
{
	int b = (int)(v * 255.0f + 0.5f);
	if (b < 0) b = 0;
	if (b > 255) b = 255;
	return b;
}

static void LD_FillColorImage(ColorImage *img, float *f, int w, bool mirror)
{
	int n = img->width * img->height;
	for (int i = 0; i < n; i++)
	{
		int s = mirror ? (i / w) * w + (w - 1 - (i % w)) : i;
		ColorImage_set_rgb(img, (size_t)i,
			(uint8_t)LD_FloatToByte(f[s * 3 + 2]),
			(uint8_t)LD_FloatToByte(f[s * 3 + 1]),
			(uint8_t)LD_FloatToByte(f[s * 3 + 0]), 255);
	}
}

static void LD_FillLuma(DitherImage *img, float *f, int w, int h, bool correctGamma, bool mirror)
{
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
		{
			int sx = mirror ? (w - 1 - x) : x;
			int i = y * w + sx;
			DitherImage_set_pixel_rgba(img, x, y,
				LD_FloatToByte(f[i * 3 + 2]),
				LD_FloatToByte(f[i * 3 + 1]),
				LD_FloatToByte(f[i * 3 + 0]), 255, correctGamma);
		}
}

// Palette indices -> float, blended toward orig by strength (per-channel gates).
// With mirror=true indices are un-mirrored back (use with mirrored fill).
static void LD_ApplyIndices(float *f, float *orig, int *idx, CachedPalette *pal,
	int n, int w, bool mirror, float strength, bool rEn, bool gEn, bool bEn)
{
	for (int i = 0; i < n; i++)
	{
		int s = mirror ? (i / w) * w + (w - 1 - (i % w)) : i;
		ByteColor *bc = BytePalette_get(pal->target_palette, (size_t)idx[i]);
		float dr = bc->r / 255.0f, dg = bc->g / 255.0f, db = bc->b / 255.0f;
		if (bEn) f[s * 3 + 0] = orig[s * 3 + 0] + (db - orig[s * 3 + 0]) * strength;
		if (gEn) f[s * 3 + 1] = orig[s * 3 + 1] + (dg - orig[s * 3 + 1]) * strength;
		if (rEn) f[s * 3 + 2] = orig[s * 3 + 2] + (dr - orig[s * 3 + 2]) * strength;
	}
}

// Deterministic gaussian pre-jitter for the COLOR path (upstream color
// ditherers expose no sigma). stddev ~= sigma * 0.08 in 0..1 units.
static void LD_JitterFloat(float *f, int n, double sigma, unsigned int seed)
{
	if (sigma <= 0.0)
		return;
	unsigned int s = seed ? seed : 0x12345678u;
	for (int i = 0; i < n * 3; i++)
	{
		s = s * 1664525u + 1013904223u;
		double u1 = ((s >> 8) & 0xffffff) / 16777216.0 + 1e-9;
		s = s * 1664525u + 1013904223u;
		double u2 = ((s >> 8) & 0xffffff) / 16777216.0;
		double g = sqrt(-2.0 * log(u1)) * cos(6.283185307179586 * u2);
		f[i] += (float)(g * sigma * 0.08);
	}
}

#endif // DITHER_BRIDGE_H
