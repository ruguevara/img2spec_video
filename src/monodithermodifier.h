class MonoDitherModifier : public Modifier
{
public:
	int mFamily;    // 0 Threshold .. 10 Mono Ordered (see combo)
	float mV;       // strength 0..2
	int mInvert;    // 0 off, 1 on
	int mMaskMode;  // 0 Modulate (hue-preserving), 1 Replace B/W
	int mGamma;     // 0 off, 1 linear (gamma-correct luma)
	int mOnce;

	int mThreshold; // 0..1000 promille (threshold/1000.0)
	float mNoise;
	int mAutoReq;   // transient: recompute threshold via auto_threshold

	int mGridW, mGridH, mGridMin, mGridAlt;

	int mPattern;

	int mDotDiff, mDotClass;

	int mLippens;

	int mVarType;   // 0 Ostromoukhov, 1 ZhouFang
	int mVarSerp;

	int mDBS;       // 0..7

	int mKallRandom;

	int mRiemCurve; // 0..7
	int mRiemOrig;  // 0 improved, 1 original Riemersma

	int mErrModel;  // 0..18
	int mErrDir;    // 0..3
	float mErrSigma;

	int mOrdMatrix; // 0..42
	float mOrdSigma;
	int mOrdVarStep, mOrdGradSize;
	float mOrdGradA, mOrdGradB, mOrdGradC;
	int mOrdXOfs, mOrdYOfs;

	DitherImage *mImg;
	uint8_t *mOut;
	int mOutN, mOutW, mOutH;

	virtual char *getname() { return "MonoDither"; }

	virtual void serialize(JSON_Object * root)
	{
		SERIALIZE(mFamily);
		SERIALIZE(mV);
		SERIALIZE(mInvert);
		SERIALIZE(mMaskMode);
		SERIALIZE(mGamma);
		SERIALIZE(mThreshold);
		SERIALIZE(mNoise);
		SERIALIZE(mGridW);
		SERIALIZE(mGridH);
		SERIALIZE(mGridMin);
		SERIALIZE(mGridAlt);
		SERIALIZE(mPattern);
		SERIALIZE(mDotDiff);
		SERIALIZE(mDotClass);
		SERIALIZE(mLippens);
		SERIALIZE(mVarType);
		SERIALIZE(mVarSerp);
		SERIALIZE(mDBS);
		SERIALIZE(mKallRandom);
		SERIALIZE(mRiemCurve);
		SERIALIZE(mRiemOrig);
		SERIALIZE(mErrModel);
		SERIALIZE(mErrDir);
		SERIALIZE(mErrSigma);
		SERIALIZE(mOrdMatrix);
		SERIALIZE(mOrdSigma);
		SERIALIZE(mOrdVarStep);
		SERIALIZE(mOrdGradSize);
		SERIALIZE(mOrdGradA);
		SERIALIZE(mOrdGradB);
		SERIALIZE(mOrdGradC);
		SERIALIZE(mOrdXOfs);
		SERIALIZE(mOrdYOfs);
	}

	virtual void deserialize(JSON_Object * root)
	{
#pragma warning(disable:4244; disable:4800)
		DESERIALIZE(mFamily);
		DESERIALIZE(mV);
		DESERIALIZE(mInvert);
		DESERIALIZE(mMaskMode);
		DESERIALIZE(mGamma);
		DESERIALIZE(mThreshold);
		DESERIALIZE(mNoise);
		DESERIALIZE(mGridW);
		DESERIALIZE(mGridH);
		DESERIALIZE(mGridMin);
		DESERIALIZE(mGridAlt);
		DESERIALIZE(mPattern);
		DESERIALIZE(mDotDiff);
		DESERIALIZE(mDotClass);
		DESERIALIZE(mLippens);
		DESERIALIZE(mVarType);
		DESERIALIZE(mVarSerp);
		DESERIALIZE(mDBS);
		DESERIALIZE(mKallRandom);
		DESERIALIZE(mRiemCurve);
		DESERIALIZE(mRiemOrig);
		DESERIALIZE(mErrModel);
		DESERIALIZE(mErrDir);
		DESERIALIZE(mErrSigma);
		DESERIALIZE(mOrdMatrix);
		DESERIALIZE(mOrdSigma);
		DESERIALIZE(mOrdVarStep);
		DESERIALIZE(mOrdGradSize);
		DESERIALIZE(mOrdGradA);
		DESERIALIZE(mOrdGradB);
		DESERIALIZE(mOrdGradC);
		DESERIALIZE(mOrdXOfs);
		DESERIALIZE(mOrdYOfs);
#pragma warning(default:4244; default:4800)
		if (mFamily < 0) mFamily = 0;
		if (mFamily > 10) mFamily = 0;
	}

	virtual int gettype()
	{
		return MOD_MONODITHER;
	}

	MonoDitherModifier()
	{
		mFamily = 0;
		mV = 1.0f;
		mInvert = 0;
		mMaskMode = 0;
		mGamma = 1;
		mOnce = 0;
		mThreshold = 500;
		mNoise = 0.55f;
		mAutoReq = 0;
		mGridW = 4; mGridH = 4; mGridMin = 0; mGridAlt = 0;
		mPattern = 0;
		mDotDiff = 0; mDotClass = 1;
		mLippens = 0;
		mVarType = 0; mVarSerp = 1;
		mDBS = 0;
		mKallRandom = 0;
		mRiemCurve = 0; mRiemOrig = 0;
		mErrModel = 0; mErrDir = 2; mErrSigma = 0;
		mOrdMatrix = 3; mOrdSigma = 0;
		mOrdVarStep = 0; mOrdGradSize = 8;
		mOrdGradA = 0.5f; mOrdGradB = 0.5f; mOrdGradC = 50.0f;
		mOrdXOfs = 0; mOrdYOfs = 0;
		mImg = 0;
		mOut = 0;
		mOutN = 0; mOutW = 0; mOutH = 0;
	}

	virtual ~MonoDitherModifier()
	{
		if (mImg) DitherImage_free(mImg);
		delete[] mOut;
	}

	virtual int ui()
	{
		int ret = 0;
		ImGui::PushID(mUnique);

		if (!mOnce)
		{
			ImGui::OpenNextNode(1);
			mOnce = 1;
		}

		if (ImGui::CollapsingHeader("Mono Dither Modifier"))
		{
			ret = common();
			complexsliderfloat("Strength", &mV, 0, 2, 1, 0.001f);

			if (ImGui::Combo("##Family  ", &mFamily,
				"Threshold\0Grid\0Pattern\0Dot Diffusion\0Dot Lippens\0"
				"Variable Error Diffusion\0DBS (slow)\0Kacker-Allebach\0"
				"Riemersma\0Error Diffusion (mono)\0Ordered (mono)\0")) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("-##family")) { gDirty = 1; mFamily = (mFamily + 11 - 1) % 11; } ImGui::SameLine();
			if (ImGui::Button("+##family")) { gDirty = 1; mFamily = (mFamily + 11 + 1) % 11; } ImGui::SameLine();
			if (ImGui::Button("Reset##family     ")) { gDirty = 1; mFamily = 0; } ImGui::SameLine();
			ImGui::Text("Family");

			if (ImGui::Combo("##MaskMode  ", &mMaskMode, "Modulate (keep hue)\0Replace B/W\0")) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##maskmode     ")) { gDirty = 1; mMaskMode = 0; } ImGui::SameLine();
			ImGui::Text("Mask apply");
			{ bool t = (mInvert != 0); if (ImGui::Checkbox("Invert mask", &t)) { mInvert = t ? 1 : 0; gDirty = 1; } }
			{ bool t = (mGamma != 0); if (ImGui::Checkbox("Linear gamma luma", &t)) { mGamma = t ? 1 : 0; gDirty = 1; } }

			switch (mFamily)
			{
			case 0:
				complexsliderint("Threshold (0-1000)", &mThreshold, 0, 1000, 500, 1);
				complexsliderfloat("Noise", &mNoise, 0, 1, 0.55f, 0.001f);
				if (ImGui::Button("Auto threshold")) { mAutoReq = 1; gDirty = 1; }
				break;
			case 1:
				complexsliderint("Grid W", &mGridW, 1, 16, 4, 1);
				complexsliderint("Grid H", &mGridH, 1, 16, 4, 1);
				complexsliderint("Min pixels", &mGridMin, 0, 128, 0, 1);
				{ bool t = (mGridAlt != 0); if (ImGui::Checkbox("Alt algorithm", &t)) { mGridAlt = t ? 1 : 0; gDirty = 1; } }
				break;
			case 2:
				if (ImGui::Combo("##Pattern  ", &mPattern, LD_PATTERN_COMBO)) { gDirty = 1; } ImGui::SameLine();
				if (ImGui::Button("Reset##pattern     ")) { gDirty = 1; mPattern = 0; } ImGui::SameLine();
				ImGui::Text("Pattern");
				break;
			case 3:
				if (ImGui::Combo("##DotDiff  ", &mDotDiff, LD_DOTDIFF_COMBO)) { gDirty = 1; } ImGui::SameLine();
				ImGui::Text("Diffusion");
				if (ImGui::Combo("##DotClass  ", &mDotClass, LD_DOTCLASS_COMBO)) { gDirty = 1; } ImGui::SameLine();
				ImGui::Text("Class matrix");
				break;
			case 4:
				if (ImGui::Combo("##Lippens  ", &mLippens, LD_LIPPENS_COMBO)) { gDirty = 1; } ImGui::SameLine();
				ImGui::Text("Coefficients");
				break;
			case 5:
				if (ImGui::Combo("##VarType  ", &mVarType, "Ostromoukhov\0Zhou Fang\0")) { gDirty = 1; } ImGui::SameLine();
				ImGui::Text("Algorithm");
				{ bool t = (mVarSerp != 0); if (ImGui::Checkbox("Serpentine", &t)) { mVarSerp = t ? 1 : 0; gDirty = 1; } }
				break;
			case 6:
				complexsliderint("DBS level", &mDBS, 0, 7, 0, 1);
				ImGui::Text("Slow on large images.");
				break;
			case 7:
				{ bool t = (mKallRandom != 0); if (ImGui::Checkbox("Random", &t)) { mKallRandom = t ? 1 : 0; gDirty = 1; } }
				break;
			case 8:
				if (ImGui::Combo("##Curve  ", &mRiemCurve, LD_CURVE_COMBO)) { gDirty = 1; } ImGui::SameLine();
				ImGui::Text("Curve");
				if (ImGui::Combo("##RiemVer  ", &mRiemOrig, "Improved\0Original Riemersma\0")) { gDirty = 1; } ImGui::SameLine();
				ImGui::Text("Variant");
				break;
			case 9:
				if (ImGui::Combo("##ErrModel  ", &mErrModel, LD_ERRDIFF_COMBO)) { gDirty = 1; } ImGui::SameLine();
				ImGui::Text("Kernel");
				if (ImGui::Combo("##ErrDir  ", &mErrDir, "Left-right\0Right-left\0Serpentine L-R first\0Serpentine R-L first\0")) { gDirty = 1; } ImGui::SameLine();
				ImGui::Text("Direction");
				complexsliderfloat("Sigma", &mErrSigma, 0, 1, 0, 0.001f);
				break;
			case 10:
				if (ImGui::Combo("##OrdMatrix  ", &mOrdMatrix, LD_ORDERED_COMBO)) { gDirty = 1; } ImGui::SameLine();
				ImGui::Text("Matrix");
				if (mOrdMatrix == LD_ORDERED_FIXED || mOrdMatrix == LD_ORDERED_FIXED + 1)
					complexsliderint("Variable step", &mOrdVarStep, 0, 100, 0, 1);
				if (mOrdMatrix == LD_ORDERED_FIXED + 2)
				{
					complexsliderint("Gradient size", &mOrdGradSize, 2, 64, 8, 1);
					complexsliderfloat("Gradient A", &mOrdGradA, 0, 1, 0.5f, 0.001f);
					complexsliderfloat("Gradient B", &mOrdGradB, 0, 1, 0.5f, 0.001f);
					complexsliderfloat("Gradient C", &mOrdGradC, 0, 100, 50, 0.01f);
				}
				complexsliderint("X Offset", &mOrdXOfs, 0, 32, 0, 1);
				complexsliderint("Y Offset", &mOrdYOfs, 0, 32, 0, 1);
				complexsliderfloat("Sigma", &mOrdSigma, 0, 0.2f, 0, 0.001f);
				break;
			}
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{
		int w = gDevice->mXRes, h = gDevice->mYRes;
		int n = w * h;
		if (n <= 0)
			return;
		if (!mImg || mOutN != n)
		{
			if (mImg) DitherImage_free(mImg);
			delete[] mOut;
			mImg = DitherImage_new(w, h);
			mOut = new uint8_t[n];
			mOutN = n; mOutW = w; mOutH = h;
		}
		// Zero every frame: some libdither writers (e.g. Riemersma below
		// threshold, unvisited curve pixels, transparent) leave out[] untouched.
		memset(mOut, 0, (size_t)n);

		// Families with traversal direction: mirror trick (upstream starts L-R).
		bool mirror = (mFamily == 9 && (mErrDir == 1 || mErrDir == 3));
		bool gamma = (mGamma != 0);
		LD_FillLuma(mImg, gBitmapProcFloat, w, h, gamma, mirror);

		if (mAutoReq && mFamily == 0)
		{
			mThreshold = (int)(auto_threshold(mImg) * 1000.0);
			mAutoReq = 0;
		}

		switch (mFamily)
		{
		case 0:
			threshold_dither(mImg, mThreshold / 1000.0, mNoise, mOut);
			break;
		case 1:
		{
			int gw = mGridW < 1 ? 1 : (mGridW > 16 ? 16 : mGridW);
			int gh = mGridH < 1 ? 1 : (mGridH > 16 ? 16 : mGridH);
			int mp = mGridMin < 0 ? 0 : mGridMin;
			if (mp > gw * gh) mp = gw * gh;
			grid_dither(mImg, gw, gh, mp, mGridAlt != 0, mOut);
			break;
		}
		case 2:
		{
			TilePattern *p = LD_PATTERNS[mPattern < 0 || mPattern >= LD_PATTERN_COUNT ? 0 : mPattern]();
			pattern_dither(mImg, p, mOut);
			TilePattern_free(p);
			break;
		}
		case 3:
		{
			DotDiffusionMatrix *d = LD_DOTDIFFS[mDotDiff < 0 || mDotDiff >= LD_DOTDIFF_COUNT ? 0 : mDotDiff]();
			DotClassMatrix *c = LD_DOTCLASS[mDotClass < 0 || mDotClass >= LD_DOTCLASS_COUNT ? 1 : mDotClass]();
			dot_diffusion_dither(mImg, d, c, mOut);
			DotDiffusionMatrix_free(d);
			DotClassMatrix_free(c);
			break;
		}
		case 4:
		{
			DotClassMatrix *c = get_dotlippens_class_matrix();
			DotLippensCoefficients *k = LD_LIPPENS[mLippens < 0 || mLippens >= LD_LIPPENS_COUNT ? 0 : mLippens]();
			dotlippens_dither(mImg, c, k, mOut);
			DotClassMatrix_free(c);
			DotLippensCoefficients_free(k);
			break;
		}
		case 5:
			variable_error_diffusion_dither(mImg,
				mVarType ? Zhoufang : Ostromoukhov, mVarSerp != 0, mOut);
			break;
		case 6:
		{
			int v = mDBS < 0 ? 0 : (mDBS > 7 ? 7 : mDBS);
			dbs_dither(mImg, v, mOut);
			break;
		}
		case 7:
			kallebach_dither(mImg, mKallRandom != 0, mOut);
			break;
		case 8:
		{
			RiemersmaCurve *c = LD_CURVES[mRiemCurve < 0 || mRiemCurve >= LD_CURVE_COUNT ? 0 : mRiemCurve]();
			riemersma_dither(mImg, c, mRiemOrig != 0, mOut);
			RiemersmaCurve_free(c);
			break;
		}
		case 9:
		{
			int md = mErrModel < 0 || mErrModel >= LD_ERRDIFF_COUNT ? 0 : mErrModel;
			ErrorDiffusionMatrix *m = LD_ERRDIFF[md].fn();
			error_diffusion_dither(mImg, m, (mErrDir >= 2), mErrSigma, mOut);
			ErrorDiffusionMatrix_free(m);
			break;
		}
		default:
		{
			int mo = mOrdMatrix < 0 || mOrdMatrix >= LD_ORDERED_COUNT ? 3 : mOrdMatrix;
			OrderedDitherMatrix *m = LD_GetOrdered(mo, mOrdVarStep, mOrdGradSize, mOrdGradA, mOrdGradB, mOrdGradC);
			OrderedDitherMatrix *s = LD_ShiftOrdered(m, mOrdXOfs, mOrdYOfs);
			ordered_dither(mImg, s ? s : m, mOrdSigma, mOut);
			if (s) OrderedDitherMatrix_free(s);
			OrderedDitherMatrix_free(m);
			break;
		}
		}

		bool inv = (mInvert != 0);
		if (mMaskMode == 0)
		{
			// Modulate: shift brightness toward mask, keep hue.
			for (int i = 0; i < n; i++)
			{
				int s = mirror ? (i / w) * w + (w - 1 - (i % w)) : i;
				float mask = (mOut[i] != 0) ? 1.0f : 0.0f;
				if (inv) mask = 1.0f - mask;
				float r = gBitmapProcFloat[s * 3 + 2];
				float g = gBitmapProcFloat[s * 3 + 1];
				float b = gBitmapProcFloat[s * 3 + 0];
				float luma = r * 0.299f + g * 0.586f + b * 0.114f;
				float d = (mask - luma) * mV;
				if (mB_en) gBitmapProcFloat[s * 3 + 0] = b + d;
				if (mG_en) gBitmapProcFloat[s * 3 + 1] = g + d;
				if (mR_en) gBitmapProcFloat[s * 3 + 2] = r + d;
			}
		}
		else
		{
			// Replace B/W: pull enabled channels toward pure mask.
			for (int i = 0; i < n; i++)
			{
				int s = mirror ? (i / w) * w + (w - 1 - (i % w)) : i;
				float mask = (mOut[i] != 0) ? 1.0f : 0.0f;
				if (inv) mask = 1.0f - mask;
				if (mB_en) gBitmapProcFloat[s * 3 + 0] += (mask - gBitmapProcFloat[s * 3 + 0]) * mV;
				if (mG_en) gBitmapProcFloat[s * 3 + 1] += (mask - gBitmapProcFloat[s * 3 + 1]) * mV;
				if (mR_en) gBitmapProcFloat[s * 3 + 2] += (mask - gBitmapProcFloat[s * 3 + 2]) * mV;
			}
		}
	}

};
