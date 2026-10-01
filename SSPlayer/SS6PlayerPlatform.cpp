#pragma warning(push)
#pragma warning(disable: 4099 4100 4189 4244 4267 4456 4458)

// 
//  SS6Platform.cpp
//
#define _HAS_STD_BYTE 0
#include "SS6PlayerPlatform.h"
#include "DxLib.h"
#include <vector>
#include <string>
#include <cstdio>

namespace ss
{
#define USE_TRIANGLE_FIN (1)
#define TEXTURE_MAX (512)

	int texture[TEXTURE_MAX];			// セルマップの参照するDxLibグラフィックハンドル
	std::string textureKey[TEXTURE_MAX];	// セルマップの参照するテクスチャキャッシュに登録するキー
	int texture_index = 0;

	static bool enableRenderingBlendFunc = false;

	int _direction;
	int _window_w;
	int _window_h;

	// マスク機能用変数
	static int g_maskScreenHandle = -1;
	static bool g_isMaskEnabled = false;

	// アプリケーション初期化時の処理
	void SSPlatformInit(void)
	{
		memset(texture, 0, sizeof(texture));
		for (int i = 0; i < TEXTURE_MAX; i++)
		{
			textureKey[i] = "";
		}
		texture_index = 0;
		_direction = PLUS_DOWN;
		_window_w = 1280;
		_window_h = 720;

		enableRenderingBlendFunc = false;

		// マスク用スクリーンの生成
		if (g_maskScreenHandle == -1)
		{
			g_maskScreenHandle = CreateMaskScreen();
		}
	}

	// アプリケーション終了時の処理
	void SSPlatformRelease(void)
	{
		for (int i = 0; i < TEXTURE_MAX; i++)
		{
			SSTextureRelease(i);
		}

		// マスク用スクリーンの削除 
		if (g_maskScreenHandle != -1)
		{
			DeleteMaskScreen();
			g_maskScreenHandle = -1;
		}
	}

	void SSSetPlusDirection(int direction, int window_w, int window_h)
	{
		_direction = direction;
		_window_w = window_w;
		_window_h = window_h;
	}

	void SSGetPlusDirection(int& direction, int& window_w, int& window_h)
	{
		direction = _direction;
		window_w = _window_w;
		window_h = _window_h;
	}

	void SSRenderingBlendFuncEnable(int flg)
	{
		enableRenderingBlendFunc = (flg != 0);
	}

	// ファイル読み込み
	unsigned char* SSFileOpen(const char* pszFileName, const char* pszMode, unsigned long* pSize, const char* pszZipFileName)
	{
		unsigned char* pBuffer = NULL;
		SS_ASSERT2(pszFileName != NULL && pSize != NULL && pszMode != NULL, "Invalid parameters.");
		*pSize = 0;

		FILE* fp = fopen(pszFileName, pszMode);
		if (fp)
		{
			fseek(fp, 0, SEEK_END);
			*pSize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			pBuffer = new unsigned char[*pSize];
			*pSize = fread(pBuffer, sizeof(unsigned char), *pSize, fp);
			fclose(fp);
		}

		if (!pBuffer)
		{
			std::string msg = "Get data from file(";
			msg.append(pszFileName).append(") failed!");
			SSLOG("%s", msg.c_str());
		}
		return pBuffer;
	}

	// テクスチャの読み込み
	long SSTextureLoad(const char* pszFileName, int wrapmode, int filtermode, const char* pszZipFileName)
	{
		long rc = -1;

		int start_index = texture_index;
		bool exit = true;
		while (exit)
		{
			if (texture[texture_index] == 0)
			{
#ifdef _UNICODE
				wchar_t wFileName[1024];
				MultiByteToWideChar(CP_UTF8, 0, pszFileName, -1, wFileName, 1024);
				int handle = LoadGraph(wFileName);
#else
				int handle = LoadGraph(pszFileName);
#endif
				if (handle == -1)
				{
					DEBUG_PRINTF("テクスチャの読み込み失敗: %s\n", pszFileName);
				}
				else
				{
					texture[texture_index] = handle;
					textureKey[texture_index] = pszFileName;
					rc = texture_index;
				}
				exit = false;
			}
			texture_index++;
			if (texture_index >= TEXTURE_MAX)
			{
				texture_index = 0;
			}
			if (texture_index == start_index)
			{
				DEBUG_PRINTF("テクスチャバッファの空きがない\n");
				exit = false;
			}
		}

		return rc;
	}

	// テクスチャの解放
	bool SSTextureRelease(long handle)
	{
		bool rc = true;
		if (handle >= 0 && handle < TEXTURE_MAX)
		{
			if (texture[handle] != 0)
			{
				DeleteGraph(texture[handle]);
				texture[handle] = 0;
			}
			else
			{
				rc = false;
			}
			textureKey[handle] = "";
		}
		return rc;
	}

	bool SSGetTextureIndex(std::string key, std::vector<int>* indexList)
	{
		bool rc = false;
		indexList->clear();

		for (int i = 0; i < TEXTURE_MAX; i++)
		{
			if (textureKey[i] == key)
			{
				indexList->push_back(i);
				rc = true;
			}
		}
		return rc;
	}

	bool SSGetTextureSize(long handle, int& w, int& h)
	{
		if (handle >= 0 && handle < TEXTURE_MAX && texture[handle] != 0)
		{
			if (GetGraphSize(texture[handle], &w, &h) == 0)
			{
				return true;
			}
		}
		return false;
	}

	struct SSDrawState
	{
		int texture;
		int partType;
		int partBlendfunc;
		int partsColorUse;
		int partsColorFunc;
		int partsColorType;
		int maskInfluence;
		void init(void)
		{
			texture = -1;
			partType = -1;
			partBlendfunc = -1;
			partsColorUse = -1;
			partsColorFunc = -1;
			partsColorType = -1;
			maskInfluence = -1;
		}
	};
	SSDrawState _ssDrawState;

	void SSRenderSetup(void) {}
	void SSRenderEnd(void)
	{
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	static void CoordinateGetDiagonalIntersection(SsVector2& out, const SsVector2& LU, const SsVector2& RU, const SsVector2& LD, const SsVector2& RD)
	{
		// 反転時や対角線が平行な場合のフォールバック（4頂点の平均）をあらかじめ設定
		out.x = (LU.x + RU.x + LD.x + RD.x) * 0.25f;
		out.y = (LU.y + RU.y + LD.y + RD.y) * 0.25f;

		float c1 = (LD.y - RU.y) * (LD.x - LU.x) - (LD.x - RU.x) * (LD.y - LU.y);
		float c2 = (RD.x - LU.x) * (LD.y - LU.y) - (RD.y - LU.y) * (LD.x - LU.x);
		float c3 = (RD.x - LU.x) * (LD.y - RU.y) - (RD.y - LU.y) * (LD.x - RU.x);

		if (c3 == 0.0f) return;

		float ca = c1 / c3;
		float cb = c2 / c3;

		// 線分上に交点が存在する場合のみ適用
		if (((0.0f <= ca) && (1.0f >= ca)) && ((0.0f <= cb) && (1.0f >= cb)))
		{
			out.x = LU.x + ca * (RD.x - LU.x);
			out.y = LU.y + ca * (RD.y - LU.y);
		}
	}

	void setClientState(SSV3F_C4B_T2F point, int index, float* uvs, float* colors, float* vertices)
	{
		uvs[0 + (index * 2)] = point.texCoords.u;
		uvs[1 + (index * 2)] = point.texCoords.v;

		colors[0 + (index * 4)] = point.colors.r / 255.0f;
		colors[1 + (index * 4)] = point.colors.g / 255.0f;
		colors[2 + (index * 4)] = point.colors.b / 255.0f;
		colors[3 + (index * 4)] = point.colors.a / 255.0f;

		vertices[0 + (index * 3)] = point.vertices.x;
		vertices[1 + (index * 3)] = point.vertices.y;
		vertices[2 + (index * 3)] = point.vertices.z;
	}

	// メッシュの表示（DxLib実装）
	void SSDrawMesh(CustomSprite* sprite, State state)
	{
		if (!sprite || sprite->_meshTriangleSize <= 0) return;

		int tex_index = state.texture.handle;
		if (tex_index < 0 || tex_index >= TEXTURE_MAX || texture[tex_index] == 0) return;

		switch (state.blendfunc)
		{
		case BLEND_MIX:      SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255); break;
		case BLEND_ADD:      SetDrawBlendMode(DX_BLENDMODE_ADD, 255); break;
		case BLEND_SUB:      SetDrawBlendMode(DX_BLENDMODE_SUB, 255); break;
		case BLEND_MUL:      SetDrawBlendMode(DX_BLENDMODE_MULA, 255); break;
		default:             SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255); break;
		}

		std::vector<VERTEX2D> verts(sprite->_meshVertexSize);
		for (size_t i = 0; i < sprite->_meshVertexSize; i++)
		{
			verts[i].pos = VGet(sprite->_mesh_vertices[i * 3 + 0], sprite->_mesh_vertices[i * 3 + 1], 0.0f);
			verts[i].u = sprite->_mesh_uvs[i * 2 + 0];
			verts[i].v = sprite->_mesh_uvs[i * 2 + 1];
			verts[i].rhw = 1.0f;
			verts[i].dif = GetColorU8(
				(BYTE)(sprite->_mesh_colors[i * 4 + 0] * 255.0f),
				(BYTE)(sprite->_mesh_colors[i * 4 + 1] * 255.0f),
				(BYTE)(sprite->_mesh_colors[i * 4 + 2] * 255.0f),
				(BYTE)(sprite->_mesh_colors[i * 4 + 3] * 255.0f)
			);
		}

		DrawPolygonIndexed2D(
			verts.data(),
			(int)sprite->_meshVertexSize,
			(WORD*)sprite->_mesh_indices,
			(int)sprite->_meshTriangleSize,
			texture[tex_index],
			TRUE
		);

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// スプライトの表示
	// スプライトの表示
	void SSDrawSprite(CustomSprite* sprite, State* overwrite_state)
	{
		if (sprite->_state.isVisibled == false) return;

		const State& state = overwrite_state ? *overwrite_state : sprite->_state;
		int tex_index = state.texture.handle;

		if (tex_index < 0 || tex_index >= TEXTURE_MAX || texture[tex_index] == 0) return;

		execMask(sprite);

		SSV3F_C4B_T2F_Quad quad = state.quad;

		float cx = state.size_X * -state.pivotX;
		float cy = state.size_Y * -state.pivotY;

		quad.tl.vertices.x += cx; quad.tl.vertices.y += cy;
		quad.tr.vertices.x += cx; quad.tr.vertices.y += cy;
		quad.bl.vertices.x += cx; quad.bl.vertices.y += cy;
		quad.br.vertices.x += cx; quad.br.vertices.y += cy;

		float mat[16];
		IdentityMatrix(mat);
		if (sprite->_parentPlayer)
		{
			const State& pls = sprite->_parentPlayer->getState();
			MultiplyMatrix((float*)pls.mat, mat, mat);
		}

		float t[16];
		TranslationMatrix(t, quad.tl.vertices.x, quad.tl.vertices.y, 0.0f);
		MultiplyMatrix(t, state.mat, t);
		MultiplyMatrix(t, mat, t);
		quad.tl.vertices.x = t[12]; quad.tl.vertices.y = t[13];

		TranslationMatrix(t, quad.tr.vertices.x, quad.tr.vertices.y, 0.0f);
		MultiplyMatrix(t, state.mat, t);
		MultiplyMatrix(t, mat, t);
		quad.tr.vertices.x = t[12]; quad.tr.vertices.y = t[13];

		TranslationMatrix(t, quad.bl.vertices.x, quad.bl.vertices.y, 0.0f);
		MultiplyMatrix(t, state.mat, t);
		MultiplyMatrix(t, mat, t);
		quad.bl.vertices.x = t[12]; quad.bl.vertices.y = t[13];

		TranslationMatrix(t, quad.br.vertices.x, quad.br.vertices.y, 0.0f);
		MultiplyMatrix(t, state.mat, t);
		MultiplyMatrix(t, mat, t);
		quad.br.vertices.x = t[12]; quad.br.vertices.y = t[13];

		float alpha = state.Calc_opacity / 255.0f;
		if (state.flags & PART_FLAG_LOCALOPACITY)
		{
			alpha = state.localopacity / 255.0f;
		}

		if (!((state.flags & PART_FLAG_PARTS_COLOR)
			&& ((VertexFlag)state.partsColorType != VertexFlag::VERTEX_FLAG_ONE)
			&& ((BlendType)state.partsColorFunc == BlendType::BLEND_MIX)))
		{
			quad.tl.colors.a = (unsigned char)(quad.tl.colors.a * alpha);
			quad.tr.colors.a = (unsigned char)(quad.tr.colors.a * alpha);
			quad.bl.colors.a = (unsigned char)(quad.bl.colors.a * alpha);
			quad.br.colors.a = (unsigned char)(quad.br.colors.a * alpha);
		}

		if (sprite->_partData.type == PARTTYPE_MESH)
		{
			SSDrawMesh(sprite, state);
			return;
		}

		switch (state.blendfunc)
		{
		case BLEND_MIX:      SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255); break;
		case BLEND_ADD:      SetDrawBlendMode(DX_BLENDMODE_ADD, 255); break;
		case BLEND_SUB:      SetDrawBlendMode(DX_BLENDMODE_SUB, 255); break;
		case BLEND_MUL:      SetDrawBlendMode(DX_BLENDMODE_MULA, 255); break;
		default:             SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255); break;
		}

		// 1. 各頂点（LU:左上, RU:右上, LD:左下, RD:右下）
		SsVector2 LU(quad.tl.vertices.x, quad.tl.vertices.y);
		SsVector2 RU(quad.tr.vertices.x, quad.tr.vertices.y);
		SsVector2 LD(quad.bl.vertices.x, quad.bl.vertices.y);
		SsVector2 RD(quad.br.vertices.x, quad.br.vertices.y);

		SsVector2 LU_uv(quad.tl.texCoords.u, quad.tl.texCoords.v);
		SsVector2 RU_uv(quad.tr.texCoords.u, quad.tr.texCoords.v);
		SsVector2 LD_uv(quad.bl.texCoords.u, quad.bl.texCoords.v);
		SsVector2 RD_uv(quad.br.texCoords.u, quad.br.texCoords.v);

		// 2. 対角線の交点を求める（反転フォールバック付き）
		SsVector2 Center, Center_uv;
		CoordinateGetDiagonalIntersection(Center, LU, RU, LD, RD);
		CoordinateGetDiagonalIntersection(Center_uv, LU_uv, RU_uv, LD_uv, RD_uv);

		// 3. 5頂点（四角形の4角 ＋ 中央の交点）を作成
		VERTEX2D v[5];

		// 左上
		v[0].pos = VGet(LU.x, LU.y, 0.0f); v[0].u = LU_uv.x; v[0].v = LU_uv.y; v[0].rhw = 1.0f;
		v[0].dif = GetColorU8(quad.tl.colors.r, quad.tl.colors.g, quad.tl.colors.b, quad.tl.colors.a);

		// 右上
		v[1].pos = VGet(RU.x, RU.y, 0.0f); v[1].u = RU_uv.x; v[1].v = RU_uv.y; v[1].rhw = 1.0f;
		v[1].dif = GetColorU8(quad.tr.colors.r, quad.tr.colors.g, quad.tr.colors.b, quad.tr.colors.a);

		// 右下
		v[2].pos = VGet(RD.x, RD.y, 0.0f); v[2].u = RD_uv.x; v[2].v = RD_uv.y; v[2].rhw = 1.0f;
		v[2].dif = GetColorU8(quad.br.colors.r, quad.br.colors.g, quad.br.colors.b, quad.br.colors.a);

		// 左下
		v[3].pos = VGet(LD.x, LD.y, 0.0f); v[3].u = LD_uv.x; v[3].v = LD_uv.y; v[3].rhw = 1.0f;
		v[3].dif = GetColorU8(quad.bl.colors.r, quad.bl.colors.g, quad.bl.colors.b, quad.bl.colors.a);

		// 中央（交点）
		v[4].pos = VGet(Center.x, Center.y, 0.0f); v[4].u = Center_uv.x; v[4].v = Center_uv.y; v[4].rhw = 1.0f;
		v[4].dif = GetColorU8(
			(quad.tl.colors.r + quad.tr.colors.r + quad.bl.colors.r + quad.br.colors.r) / 4,
			(quad.tl.colors.g + quad.tr.colors.g + quad.bl.colors.g + quad.br.colors.g) / 4,
			(quad.tl.colors.b + quad.tr.colors.b + quad.bl.colors.b + quad.br.colors.b) / 4,
			(quad.tl.colors.a + quad.tr.colors.a + quad.bl.colors.a + quad.br.colors.a) / 4
		);

		// 4. 4つの三角形を構成するインデックス（12個）を作成
		WORD indices[12] = {
			0, 1, 4,  // 上三角形
			1, 2, 4,  // 右三角形
			2, 3, 4,  // 下三角形
			3, 0, 4   // 左三角形
		};

		// 5. 裏返り（カリング）対策を行って描画呼び出し
		SetUseBackCulling(FALSE);
		DrawPolygonIndexed2D(v, 5, indices, 4, texture[tex_index], TRUE);
		SetUseBackCulling(TRUE);
	}

	// --- マスク処理の実装 ---
	void clearMask()
	{
		if (g_maskScreenHandle != -1)
		{
			FillMaskScreen(0);
		}
	}

	void enableMask(bool flag)
	{
		g_isMaskEnabled = flag;
		SetUseMaskScreenFlag(flag ? TRUE : FALSE);
	}

	void execMask(CustomSprite* sprite)
	{
		if (sprite && sprite->_partData.type == PARTTYPE_MASK)
		{
			enableMask(true);
		}
	}

	std::string utf8Togbk(const char* src)
	{
		int len = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
		unsigned short* wszGBK = new unsigned short[len + 1];
		memset(wszGBK, 0, len * 2 + 2);
		MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)src, -1, (LPWSTR)wszGBK, len);

		len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
		char* szGBK = new char[len + 1];
		memset(szGBK, 0, len + 1);
		WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, szGBK, len, NULL, NULL);
		std::string strTemp(szGBK);
		if (strTemp.find('?') != std::string::npos)
		{
			strTemp.assign(src);
		}
		delete[] szGBK;
		delete[] wszGBK;
		return strTemp;
	}

	bool isAbsolutePath(const std::string& strPath)
	{
		std::string strPathAscii = utf8Togbk(strPath.c_str());
		if (strPathAscii.length() > 2
			&& ((strPathAscii[0] >= 'a' && strPathAscii[0] <= 'z') || (strPathAscii[0] >= 'A' && strPathAscii[0] <= 'Z'))
			&& strPathAscii[1] == ':')
		{
			return true;
		}
		return false;
	}
}

#pragma warning(pop)
