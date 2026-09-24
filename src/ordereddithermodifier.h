class OrderedDitherModifier : public Modifier
{
public:
	float mV;       // strength 0..2 (additive threshold amplitude scale)
	int mOnce;
	int mXOfs, mYOfs;
	int mMatrix;    // 0..42, LD_ORDERED (legacy 0..4 map to Bayer 2/3/3/4/8)
	float mSigma;   // pre-jitter 0..0.2 (bridge extension: upstream color has no sigma)
	int mCompare;   // 0..9, LD_COMPARE (color distance)
	int mSeed;      // jitter seed (deterministic; 0 = fixed default)
	int mVarStep;   // 0..100, Variable 2x2/4x4
	int mGradSize;  // interleaved gradient size
	float mGradA, mGradB, mGradC; // interleaved gradient params

	CachedPalette *mPal;
	char mPalDev[64];
	int mPalCount, mPalMode;
	ColorImage *mImg;
	int *mOut;
	float *mWork;
	int mWorkN;

	virtual char *getname() { return "OrderedDither"; }

	virtual void serialize(JSON_Object * root)
	{
		SERIALIZE(mV);
		SERIALIZE(mXOfs);
		SERIALIZE(mYOfs);
		SERIALIZE(mMatrix);
		SERIALIZE(mSigma);
		SERIALIZE(mCompare);
		SERIALIZE(mSeed);
		SERIALIZE(mVarStep);
		SERIALIZE(mGradSize);
		SERIALIZE(mGradA);
		SERIALIZE(mGradB);
		SERIALIZE(mGradC);
	}

	virtual void deserialize(JSON_Object * root)
	{
#pragma warning(disable:4244; disable:4800)
		DESERIALIZE(mV);
		DESERIALIZE(mXOfs);
		DESERIALIZE(mYOfs);
		DESERIALIZE(mMatrix);
		DESERIALIZE(mSigma);
		DESERIALIZE(mCompare);
		DESERIALIZE(mSeed);
		DESERIALIZE(mVarStep);
		DESERIALIZE(mGradSize);
		DESERIALIZE(mGradA);
		DESERIALIZE(mGradB);
		DESERIALIZE(mGradC);
#pragma warning(default:4244; default:4800)
		// Legacy 0..4 (2x2, 3x3, 3x3alt, 4x4, 8x8) land on Bayer
		// 2/3/3/4/8 automatically (same indices in the new registry).
		if (mMatrix < 0) mMatrix = 0;
		if (mMatrix > LD_ORDERED_COUNT - 1) mMatrix = LD_ORDERED_COUNT - 1;
		if (mCompare < 0) mCompare = 0;
		if (mCompare > LD_COMPARE_COUNT - 1) mCompare = LD_COMPARE_COUNT - 1;
	}

	virtual int gettype()
	{
		return MOD_ORDEREDDITHER;
	}

	OrderedDitherModifier()
	{
		mV = 1.0f;
		mOnce = 0;
		mXOfs = 0;
		mYOfs = 0;
		mMatrix = 3; // Bayer 8x8 (closest to old default index 4 visually)
		mSigma = 0.0f;
		mCompare = 1; // sRGB
		mSeed = 0;
		mVarStep = 0;
		mGradSize = 8;
		mGradA = 0.5f;
		mGradB = 0.5f;
		mGradC = 50.0f;
		mPal = 0;
		mPalDev[0] = 0;
		mPalCount = -1;
		mPalMode = -1;
		mImg = 0;
		mOut = 0;
		mWork = 0;
		mWorkN = 0;
	}

	virtual ~OrderedDitherModifier()
	{
		LD_FreePalette(mPal);
		if (mImg) ColorImage_free(mImg);
		delete[] mOut;
		delete[] mWork;
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

		if (ImGui::CollapsingHeader("Ordered Dither Modifier"))
		{
			ret = common();
			complexsliderfloat("Strength", &mV, 0, 2, 1, 0.001f);

			if (ImGui::Combo("##Matrix  ", &mMatrix, LD_ORDERED_COMBO)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("-##matrix")) { gDirty = 1; mMatrix = (mMatrix + LD_ORDERED_COUNT - 1) % LD_ORDERED_COUNT; } ImGui::SameLine();
			if (ImGui::Button("+##matrix")) { gDirty = 1; mMatrix = (mMatrix + LD_ORDERED_COUNT + 1) % LD_ORDERED_COUNT; } ImGui::SameLine();
			if (ImGui::Button("Reset##matrix     ")) { gDirty = 1; mMatrix = 3; } ImGui::SameLine();
			ImGui::Text("Matrix");

			if (mMatrix == LD_ORDERED_FIXED || mMatrix == LD_ORDERED_FIXED + 1)
			{
				complexsliderint("Variable step", &mVarStep, 0, 100, 0, 1);
			}
			if (mMatrix == LD_ORDERED_FIXED + 2)
			{
				complexsliderint("Gradient size", &mGradSize, 2, 64, 8, 1);
				complexsliderfloat("Gradient A", &mGradA, 0, 1, 0.5f, 0.001f);
				complexsliderfloat("Gradient B", &mGradB, 0, 1, 0.5f, 0.001f);
				complexsliderfloat("Gradient C", &mGradC, 0, 100, 50, 0.01f);
			}

			complexsliderint("X Offset", &mXOfs, 0, 32, 0, 1);
			complexsliderint("Y Offset", &mYOfs, 0, 32, 0, 1);

			complexsliderfloat("Jitter (sigma)", &mSigma, 0, 0.2f, 0, 0.001f);
			complexsliderint("Jitter seed", &mSeed, 0, 9999, 0, 1);

			if (ImGui::Combo("##Compare  ", &mCompare, LD_COMPARE_COMBO)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("-##compare")) { gDirty = 1; mCompare = (mCompare + LD_COMPARE_COUNT - 1) % LD_COMPARE_COUNT; } ImGui::SameLine();
			if (ImGui::Button("+##compare")) { gDirty = 1; mCompare = (mCompare + LD_COMPARE_COUNT + 1) % LD_COMPARE_COUNT; } ImGui::SameLine();
			if (ImGui::Button("Reset##compare     ")) { gDirty = 1; mCompare = 1; } ImGui::SameLine();
			ImGui::Text("Color distance");
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
		if (!mWork || mWorkN != n)
		{
			delete[] mWork;
			delete[] mOut;
			if (mImg) ColorImage_free(mImg);
			mWork = new float[n * 3];
			mOut = new int[n];
			mImg = ColorImage_new(w, h);
			mWorkN = n;
		}
		if (mMatrix < 0 || mMatrix >= LD_ORDERED_COUNT) mMatrix = 3;

		memcpy(mWork, gBitmapProcFloat, sizeof(float) * n * 3);
		LD_JitterFloat(mWork, n, mSigma, (unsigned int)mSeed);
		LD_FillColorImage(mImg, mWork, w, false);
		LD_EnsurePalette(mPal, mPalDev, 64, mPalCount, mPalMode, gDevice, mCompare);

		OrderedDitherMatrix *m = LD_GetOrdered(mMatrix, mVarStep, mGradSize, mGradA, mGradB, mGradC);
		OrderedDitherMatrix *s = LD_ShiftOrdered(m, mXOfs, mYOfs);
		ordered_dither_color(mImg, mPal, s ? s : m, mOut);
		if (s) OrderedDitherMatrix_free(s);
		OrderedDitherMatrix_free(m);
		CachedPalette_free_cache(mPal);

		LD_ApplyIndices(gBitmapProcFloat, mWork, mOut, mPal, n, w, false, mV, mR_en, mG_en, mB_en);
	}
};
