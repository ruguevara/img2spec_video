class ErrorDiffusionDitherModifier : public Modifier
{
public:
	float mV;        // strength 0..2
	int mModel;      // 0..18, LD_ERRDIFF (0..6 keep legacy order)
	int mDirection;  // 0 L-R, 1 R-L, 2 bidir L-first, 3 bidir R-first
	float mSigma;    // pre-jitter 0..1 (bridge extension: upstream color has no sigma)
	int mCompare;    // 0..9, LD_COMPARE (color distance)
	int mSeed;       // jitter seed (deterministic; 0 = fixed default)
	int mOnce;

	CachedPalette *mPal;
	char mPalDev[64];
	int mPalCount, mPalMode;
	ColorImage *mImg;
	int *mOut;
	float *mWork;
	int mWorkN;

	virtual char *getname() { return "ErrorDiffusionDither"; }

	virtual void serialize(JSON_Object * root)
	{
		SERIALIZE(mV);
		SERIALIZE(mModel);
		SERIALIZE(mDirection);
		SERIALIZE(mSigma);
		SERIALIZE(mCompare);
		SERIALIZE(mSeed);
	}

	virtual void deserialize(JSON_Object * root)
	{
#pragma warning(disable:4244; disable:4800)
		DESERIALIZE(mV);
		DESERIALIZE(mModel);
		DESERIALIZE(mDirection);
		DESERIALIZE(mSigma);
		DESERIALIZE(mCompare);
		DESERIALIZE(mSeed);
#pragma warning(default:4244; default:4800)
		// NOTE: legacy mErrorClamp key (pre-libdither) is ignored on purpose.
		if (mModel < 0) mModel = 0;
		if (mModel > LD_ERRDIFF_COUNT - 1) mModel = LD_ERRDIFF_COUNT - 1;
		if (mCompare < 0) mCompare = 0;
		if (mCompare > LD_COMPARE_COUNT - 1) mCompare = LD_COMPARE_COUNT - 1;
	}

	virtual int gettype()
	{
		return MOD_ERRORDIFFUSION;
	}

	ErrorDiffusionDitherModifier()
	{
		mV = 1;
		mModel = 0;
		mOnce = 0;
		mDirection = 0;
		mSigma = 0;
		mCompare = 1; // sRGB
		mSeed = 0;
		mPal = 0;
		mPalDev[0] = 0;
		mPalCount = -1;
		mPalMode = -1;
		mImg = 0;
		mOut = 0;
		mWork = 0;
		mWorkN = 0;
	}

	virtual ~ErrorDiffusionDitherModifier()
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

		if (ImGui::CollapsingHeader("Error Diffusion Dither Modifier"))
		{
			ret = common();

			complexsliderfloat("Strength", &mV, 0, 2, 1, 0.001f);
			if (ImGui::Combo("##Model  ", &mModel, LD_ERRDIFF_COMBO)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("-##model")) { gDirty = 1; mModel = (mModel + LD_ERRDIFF_COUNT - 1) % LD_ERRDIFF_COUNT; } ImGui::SameLine();
			if (ImGui::Button("+##model")) { gDirty = 1; mModel = (mModel + LD_ERRDIFF_COUNT + 1) % LD_ERRDIFF_COUNT; } ImGui::SameLine();
			if (ImGui::Button("Reset##model     ")) { gDirty = 1; mModel = 0; } ImGui::SameLine();
			ImGui::Text("Model");

			if (ImGui::Combo("##Direction  ", &mDirection, "Left-right\0Right-left\0Bidirectional, left-right first\0Bidirectional, right-left first\0")) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("-##Direction")) { gDirty = 1; mDirection = (mDirection + 4 - 1) % 4; } ImGui::SameLine();
			if (ImGui::Button("+##Direction")) { gDirty = 1; mDirection = (mDirection + 4 + 1) % 4; } ImGui::SameLine();
			if (ImGui::Button("Reset##Direction     ")) { gDirty = 1; mDirection = 0; } ImGui::SameLine();
			ImGui::Text("Direction");

			complexsliderfloat("Jitter (sigma)", &mSigma, 0, 1, 0, 0.001f);
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
		if (mModel < 0 || mModel >= LD_ERRDIFF_COUNT) mModel = 0;

		memcpy(mWork, gBitmapProcFloat, sizeof(float) * n * 3);
		LD_JitterFloat(mWork, n, mSigma, (unsigned int)mSeed);
		bool mirror = (mDirection == 1 || mDirection == 3);
		bool serpentine = (mDirection >= 2);
		LD_FillColorImage(mImg, mWork, w, mirror);
		LD_EnsurePalette(mPal, mPalDev, 64, mPalCount, mPalMode, gDevice, mCompare);

		ErrorDiffusionMatrix *m = LD_ERRDIFF[mModel].fn();
		error_diffusion_dither_color(mImg, m, mPal, serpentine, mOut);
		ErrorDiffusionMatrix_free(m);
		CachedPalette_free_cache(mPal);

		LD_ApplyIndices(gBitmapProcFloat, mWork, mOut, mPal, n, w, mirror, mV, mR_en, mG_en, mB_en);
	}

};
