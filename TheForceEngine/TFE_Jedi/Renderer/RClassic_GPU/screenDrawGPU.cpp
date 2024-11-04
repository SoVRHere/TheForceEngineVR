#include <TFE_System/profiler.h>
#include <TFE_System/math.h>
#include <TFE_Jedi/Math/fixedPoint.h>
#include <TFE_Jedi/Math/core_math.h>
#include <TFE_Jedi/Renderer/jediRenderer.h>
#include <TFE_RenderBackend/renderBackend.h>
#include <TFE_RenderBackend/shader.h>
#include <TFE_RenderBackend/vertexBuffer.h>
#include <TFE_RenderBackend/indexBuffer.h>
#include <TFE_RenderShared/lineDraw2d.h>
#include <TFE_RenderShared/quadDraw2d.h>
#include <TFE_RenderShared/texturePacker.h>
#include <TFE_Settings/settings.h>
#include <TFE_Vr/vr.h>

#include "screenDrawGPU.h"
#include "rsectorGPU.h"
#include <algorithm>
#include <cstring>

using namespace TFE_RenderBackend;

namespace TFE_RenderBackend
{
	extern Mat4  s_cameraProjVR[2];
	extern Mat4  s_cameraProjVR_YDown[2];
	extern Mat3  s_cameraMtxVR[2];
	extern Mat3  s_cameraMtxVR_YDown[2];
	extern Vec3f s_cameraPosVR[2];
	extern Vec3f s_cameraPosVR_YDown[2];
}

namespace TFE_Jedi
{
	const Vec4f* ShiftVR = nullptr;
	bool LockToCameraVR = false;

	enum Const
	{
		MAX_INDEXED_COLORS = 8,
	};

	static bool s_initialized = false;
	static s32 s_indexColorCount = 0;
	static Vec4f s_indexedColors[MAX_INDEXED_COLORS];
	extern TextureGpu* s_trueColorMapping;

	struct ScreenQuadVertex
	{
		Vec4f posUv;			// position (xy) and texture coordinates (zw)
		Vec4f shift;			// [x, y, z] shift after projection, [w] distance to projection plane
		s32	lockToCamera;		// lock to camera so it's always facing the camera, if > 0 then VR HmdView is uset to transform projected quads
		u32 textureId_Color;	// packed: rg = textureId or 0xffff; b = color index; a = light level.
	};

	static const AttributeMapping c_quadAttrMapping[] =
	{
		{ATTR_POS,   ATYPE_FLOAT, 4, 0, false},
		{ATTR_UV,    ATYPE_FLOAT, 4, 0, false},
		{ATTR_UV1,   ATYPE_INT32, 1, 0, false},
		{ATTR_COLOR, ATYPE_UINT8, 4, 0, true}
	};
	static const u32 c_quadAttrCount = TFE_ARRAYSIZE(c_quadAttrMapping);

	static VertexBuffer s_scrQuadVb;	// Dynamic vertex buffer.
	static IndexBuffer  s_scrQuadIb;	// Static index buffer.
	static Shader       s_scrQuadShader;
	static ScreenQuadVertex* s_scrQuads = nullptr;

	static s32 s_screenQuadCount = 0;
	static s32 s_svScaleOffset = -1;
	static s32 s_sIndexedColors = -1;
	static s32 s_palFxLumMask = -1;
	static s32 s_palFxFlash = -1;
	static u32 s_scrQuadsWidth;
	static u32 s_scrQuadsHeight;

	// VR specific
	static s32 s_cameraPosId = -1;
	static s32 s_cameraViewId = -1;
	static s32 s_cameraProjId = -1;
	static const ShaderUniform* s_cameraPosUniforms{ nullptr };
	static const ShaderUniform* s_cameraViewUniforms{ nullptr };
	static const ShaderUniform* s_cameraProjUniforms{ nullptr };
	static s32 s_screenSizePosId;
	static s32 s_frustumId;
	static s32 s_HmdViewId;

	struct ShaderSettingsSDGPU
	{
		bool bloom = false;
		bool trueColor = false;
		bool vr = false;
		bool vrMultiview = false;
	};
	static ShaderSettingsSDGPU s_shaderSettings = {};

	enum Constants
	{
		SCR_MAX_QUAD_COUNT = 4096,
	};

	bool screenGPU_loadShaders()
	{
		u32 defineCount = 0;
		ShaderDefine defines[16];
		if (s_shaderSettings.bloom)
		{
			defines[defineCount].name = "OPT_BLOOM";
			defines[defineCount].value = "1";
			defineCount++;
		}
		if (s_shaderSettings.trueColor)
		{
			defines[defineCount].name = "OPT_TRUE_COLOR";
			defines[defineCount].value = "1";
			defineCount++;
		}
		if (s_shaderSettings.vr)
		{
			defines[defineCount++] = { "OPT_VR", "1" };
			if (s_shaderSettings.vrMultiview)
			{
				defines[defineCount++] = { "OPT_VR_MULTIVIEW", "1" };
			}
		}

		if (!s_scrQuadShader.load("Shaders/gpu_render_quad.vert", "Shaders/gpu_render_quad.frag", defineCount, defines, SHADER_VER_STD))
		{
			return false;
		}

		// VR specific
		s_cameraPosId = s_scrQuadShader.getVariableId("CameraPos", &s_cameraPosUniforms);
		s_cameraViewId = s_scrQuadShader.getVariableId("CameraView", &s_cameraViewUniforms);
		s_cameraProjId = s_scrQuadShader.getVariableId("CameraProj", &s_cameraProjUniforms);
		s_screenSizePosId = s_scrQuadShader.getVariableId("ScreenSize");
		s_frustumId = s_scrQuadShader.getVariableId("Frustum");
		s_HmdViewId = s_scrQuadShader.getVariableId("HmdView");

		s_svScaleOffset = s_scrQuadShader.getVariableId("ScaleOffset");
		if (s_svScaleOffset < 0)
		{
			//return false;
		}

		s_sIndexedColors = s_scrQuadShader.getVariableId("IndexedColors");
		s_palFxLumMask = s_scrQuadShader.getVariableId("PalFxLumMask");
		s_palFxFlash = s_scrQuadShader.getVariableId("PalFxFlash");

		s_scrQuadShader.bindTextureNameToSlot("Colormap",     0);
		s_scrQuadShader.bindTextureNameToSlot("Palette",      1);
		s_scrQuadShader.bindTextureNameToSlot("Textures",     2);
		s_scrQuadShader.bindTextureNameToSlot("TextureTable", 3);

		return true;
	}

	void screenGPU_init()
	{
		if (!s_initialized)
		{
			TFE_RenderShared::init();
			TFE_RenderShared::quadInit();

			// Create the index and vertex buffer for quads.
			s_scrQuadVb.create(SCR_MAX_QUAD_COUNT * 4, sizeof(ScreenQuadVertex), c_quadAttrCount, c_quadAttrMapping, true);
			u16 indices[SCR_MAX_QUAD_COUNT * 6];
			u16* idx = indices;
			for (s32 q = 0, vtx = 0; q < SCR_MAX_QUAD_COUNT; q++, idx += 6, vtx += 4)
			{
				idx[0] = vtx + 0;
				idx[1] = vtx + 1;
				idx[2] = vtx + 2;

				idx[3] = vtx + 0;
				idx[4] = vtx + 2;
				idx[5] = vtx + 3;
			}
			s_scrQuadIb.create(SCR_MAX_QUAD_COUNT * 6, sizeof(u16), false, indices);
			s_scrQuads = (ScreenQuadVertex*)malloc(sizeof(ScreenQuadVertex) * SCR_MAX_QUAD_COUNT * 4);
			s_screenQuadCount = 0;

			// Shaders and variables.
			s_shaderSettings.bloom = TFE_Settings::getGraphicsSettings()->bloomEnabled;
			s_shaderSettings.trueColor = TFE_Settings::getGraphicsSettings()->colorMode == COLORMODE_TRUE_COLOR;
			s_shaderSettings.vr = TFE_Settings::getTempSettings()->vr;
			s_shaderSettings.vrMultiview = TFE_Settings::getTempSettings()->vrMultiview;
			screenGPU_loadShaders();
		}
		s_initialized = true;
	}

	void screenGPU_destroy()
	{
		if (s_initialized)
		{
			TFE_RenderShared::destroy();
			TFE_RenderShared::quadDestroy();
			s_scrQuadVb.destroy();
			s_scrQuadIb.destroy();
			free(s_scrQuads);
		}
		s_scrQuads = nullptr;
		s_initialized = false;
		s_shaderSettings = {};
	}

	void screenGPU_setHudTextureCallbacks(s32 count, TextureListCallback* callbacks, bool forceAllocation)
	{
		if (count)
		{
			TexturePacker* texturePacker = texturepacker_getGlobal();
			if (forceAllocation)
			{
				texturepacker_reset();
			}

			if (!texturepacker_hasReservedPages(texturePacker))
			{
				for (s32 i = 0; i < count; i++)
				{
					texturepacker_pack(callbacks[i], POOL_GAME);
				}
				texturepacker_commit();
				texturepacker_reserveCommitedPages(texturePacker);
			}
		}
	}

	void screenGPU_setIndexedColors(u32 count, const Vec4f* colors)
	{
		s_indexColorCount = std::min((u32)MAX_INDEXED_COLORS, count);
		memcpy(s_indexedColors, colors, sizeof(Vec4f)*s_indexColorCount);
	}
		
	void screenGPU_beginQuads(u32 width, u32 height)
	{
		s_screenQuadCount = 0;
		s_scrQuadsWidth = width;
		s_scrQuadsHeight = height;

		// Update the shaders if needed.
		bool bloomEnabled = TFE_Settings::getGraphicsSettings()->bloomEnabled;
		bool trueColorEnabled = TFE_Settings::getGraphicsSettings()->colorMode == COLORMODE_TRUE_COLOR;
		if (s_shaderSettings.bloom != bloomEnabled || s_shaderSettings.trueColor != trueColorEnabled ||
			s_shaderSettings.vr != TFE_Settings::getTempSettings()->vr ||
			s_shaderSettings.vrMultiview != TFE_Settings::getTempSettings()->vrMultiview)
		{
			s_shaderSettings.bloom = bloomEnabled;
			s_shaderSettings.trueColor = trueColorEnabled;
			s_shaderSettings.vr = TFE_Settings::getTempSettings()->vr;
			s_shaderSettings.vrMultiview = TFE_Settings::getTempSettings()->vrMultiview;
			screenGPU_loadShaders();
		}
	}

	void screenGPU_endQuads()
	{
		if (s_screenQuadCount)
		{
			s_scrQuadVb.update(s_scrQuads, sizeof(ScreenQuadVertex) * s_screenQuadCount * 4);
			TFE_RenderState::setStateEnable(false, STATE_CULLING | STATE_DEPTH_TEST | STATE_DEPTH_WRITE | STATE_BLEND);
			
			s_scrQuadShader.bind();
			s_scrQuadVb.bind();
			s_scrQuadIb.bind();

			// Bind Uniforms & Textures.
			const f32 scaleX = 2.0f / f32(s_scrQuadsWidth);
			const f32 scaleY = 2.0f / f32(s_scrQuadsHeight);
			const f32 offsetX = -1.0f;
			const f32 offsetY = -1.0f;

			const f32 scaleOffset[] = { scaleX, scaleY, offsetX, offsetY };
			s_scrQuadShader.setVariable(s_svScaleOffset, SVT_VEC4, scaleOffset);

			if (s_sIndexedColors >= 0)
			{
				s_scrQuadShader.setVariableArray(s_sIndexedColors, SVT_VEC4, (f32*)s_indexedColors, s_indexColorCount);
			}
			if (s_palFxLumMask >= 0 && s_palFxFlash >= 0)
			{
				Vec3f lumMask, palFx;
				renderer_getPalFx(&lumMask, &palFx);

				s_scrQuadShader.setVariable(s_palFxLumMask, SVT_VEC3, lumMask.m);
				s_scrQuadShader.setVariable(s_palFxFlash, SVT_VEC3, palFx.m);
			}

			if (TFE_Settings::getTempSettings()->vr)
			{
				const std::array<Vec3f, 8>& frustum = vr::GetUnitedFrustum();
				const Mat3 hmdMtx = TFE_Math::getMatrix3(vr::GetEyePose(vr::Side::Left).mTransformationYDown);

				(s_cameraPosUniforms && s_cameraPosUniforms->size > 1) ?
					s_scrQuadShader.setVariableArray(s_cameraPosId, SVT_VEC3, s_cameraPosVR_YDown[0].m, 2) :
					s_scrQuadShader.setVariable(s_cameraPosId, SVT_VEC3, s_cameraPosVR_YDown[0].m);
				(s_cameraViewUniforms && s_cameraViewUniforms->size > 1) ?
					s_scrQuadShader.setVariableArray(s_cameraViewId, SVT_MAT3x3, s_cameraMtxVR_YDown[0].data, 2) :
					s_scrQuadShader.setVariable(s_cameraViewId, SVT_MAT3x3, s_cameraMtxVR_YDown[0].data);
				(s_cameraProjUniforms && s_cameraProjUniforms->size > 1) ?
					s_scrQuadShader.setVariableArray(s_cameraProjId, SVT_MAT4x4, s_cameraProjVR_YDown[0].data, 2) :
					s_scrQuadShader.setVariable(s_cameraProjId, SVT_MAT4x4, s_cameraProjVR_YDown[0].data);
				s_scrQuadShader.setVariable(s_screenSizePosId, SVT_VEC2, Vec2f{ f32(s_scrQuadsWidth), f32(s_scrQuadsHeight) }.m);
				s_scrQuadShader.setVariableArray(s_frustumId, SVT_VEC3, frustum.data()->m, (u32)frustum.size());
				s_scrQuadShader.setVariable(s_HmdViewId, SVT_MAT3x3, hmdMtx.data);
			}

			const TextureGpu* palette  = TFE_RenderBackend::getPaletteTexture();
			const TextureGpu* colormap = TFE_Sectors_GPU::getColormap();
			colormap->bind(0);
			if (s_shaderSettings.trueColor)
			{
				s_trueColorMapping->bind(1);
			}
			else
			{
				palette->bind(1);
			}

			TexturePacker* texturePacker = texturepacker_getGlobal();
			texturePacker->texture->bind(2);
			texturePacker->textureTableGPU.bind(3);

			TFE_RenderBackend::drawIndexedTriangles(2 * s_screenQuadCount, sizeof(u16));

			s_scrQuadVb.unbind();
			s_scrQuadShader.unbind();
		}
	}
		
	void screenGPU_beginLines(u32 width, u32 height)
	{
		TFE_RenderShared::lineDraw2d_begin(width, height);
	}

	void screenGPU_endLines()
	{
		TFE_RenderShared::lineDraw2d_drawLines();
	}
		
	void screenGPU_beginImageQuads(u32 width, u32 height)
	{
		TFE_RenderShared::quadDraw2d_begin(width, height);
	}

	void screenGPU_endImageQuads()
	{
		TFE_RenderShared::quadDraw2d_draw();
	}

	void screenGPU_addImageQuad(s32 x0, s32 z0, s32 x1, s32 z1, TextureGpu* texture)
	{
		u32 colors[] = { 0xffffffff, 0xffffffff };
		Vec2f vtx[] = { f32(x0), f32(z0), f32(x1), f32(z1) };
		TFE_RenderShared::quadDraw2d_add(vtx, colors, texture);
	}

	void screenGPU_addImageQuad(s32 x0, s32 z0, s32 x1, s32 z1, f32 u0, f32 u1, TextureGpu* texture)
	{
		u32 colors[] = { 0xffffffff, 0xffffffff };
		Vec2f vtx[] = { f32(x0), f32(z0), f32(x1), f32(z1) };
		TFE_RenderShared::quadDraw2d_add(vtx, colors, u0, u1, texture);
	}

	void screenGPU_drawPoint(ScreenRect* rect, s32 x, s32 z, u8 color)
	{
		u32 color32 = vfb_getPalette()[color];
		u32 colors[] = { color32, color32 };

		// Draw a diamond shape
		f32 size = max(f32(s_scrQuadsHeight / 400), 0.5f);
		const Vec2f vtx[]=
		{
			// left
			f32(x-size), f32(z),
			// middle
			f32(x), f32(z+size),
			// right
			f32(x+size), f32(z),
			// bottom
			f32(x), f32(z-size),
			// left
			f32(x-size), f32(z),
		};
		TFE_RenderShared::lineDraw2d_addLine(1.5f, &vtx[0], colors);
		TFE_RenderShared::lineDraw2d_addLine(1.5f, &vtx[1], colors);
		TFE_RenderShared::lineDraw2d_addLine(1.5f, &vtx[2], colors);
		TFE_RenderShared::lineDraw2d_addLine(1.5f, &vtx[3], colors);
	}

	void screenGPU_drawLine(ScreenRect* rect, s32 x0, s32 z0, s32 x1, s32 z1, u8 color)
	{
		u32 color32 = vfb_getPalette()[color];
		u32 colors[] = { color32, color32 };
		Vec2f vtx[] = { f32(x0), f32(z0), f32(x1), f32(z1) };

		TFE_RenderShared::lineDraw2d_addLine(1.5f, vtx, colors);
	}

	void screenGPU_blitTexture(TextureData* texture, DrawRect* rect, s32 x0, s32 y0, JBool forceTransparency, JBool forceOpaque)
	{
		if (s_screenQuadCount >= SCR_MAX_QUAD_COUNT)
		{
			return;
		}

		u32 textureId_Color = texture->textureId;
		ScreenQuadVertex* quad = &s_scrQuads[s_screenQuadCount * 4];
		s_screenQuadCount++;

		f32 fx0 = f32(x0);
		f32 fy0 = f32(y0);
		f32 fx1 = f32(x0 + texture->width);
		f32 fy1 = f32(y0 + texture->height);

		f32 u0 = 0.0f, v0 = 0.0f;
		f32 u1 = f32(texture->width);
		f32 v1 = f32(texture->height);
		quad[0].posUv = { fx0, fy0, u0, v1 };
		quad[1].posUv = { fx1, fy0, u1, v1 };
		quad[2].posUv = { fx1, fy1, u1, v0 };
		quad[3].posUv = { fx0, fy1, u0, v0 };

		const Vec4f shift = ShiftVR ? Vec4f{ ShiftVR->x, -ShiftVR->y, ShiftVR->z, ShiftVR->w } : Vec4f{ 0.0f, 0.0f, 0.0f, 0.0f };
		quad[0].shift = quad[1].shift = quad[2].shift = quad[3].shift = shift;

		quad[0].lockToCamera = quad[1].lockToCamera = quad[2].lockToCamera = quad[3].lockToCamera = LockToCameraVR ? 1 : 0;

		quad[0].textureId_Color = textureId_Color;
		quad[1].textureId_Color = textureId_Color;
		quad[2].textureId_Color = textureId_Color;
		quad[3].textureId_Color = textureId_Color;
	}

	void screenGPU_blitTextureLit(TextureData* texture, DrawRect* rect, s32 x0, s32 y0, u8 lightLevel, JBool forceTransparency)
	{
		if (s_screenQuadCount >= SCR_MAX_QUAD_COUNT)
		{
			return;
		}

		fixed16_16 x1 = x0 + intToFixed16(texture->width);
		fixed16_16 y1 = y0 + intToFixed16(texture->height);
		s32 textureId = texture->textureId;

		u8 color = 0;
		u32 textureId_Color = textureId | (u32(color) << 16u) | (u32(lightLevel) << 24u);

		ScreenQuadVertex* quad = &s_scrQuads[s_screenQuadCount * 4];
		s_screenQuadCount++;

		f32 fx0 = fixed16ToFloat(x0);
		f32 fy0 = fixed16ToFloat(y0);
		f32 fx1 = fixed16ToFloat(x1);
		f32 fy1 = fixed16ToFloat(y1);

		f32 u0 = 0.0f, v0 = 0.0f;
		f32 u1 = f32(texture->width);
		f32 v1 = f32(texture->height);
		quad[0].posUv = { fx0, fy0, u0, v1 };
		quad[1].posUv = { fx1, fy0, u1, v1 };
		quad[2].posUv = { fx1, fy1, u1, v0 };
		quad[3].posUv = { fx0, fy1, u0, v0 };

		const Vec4f shift = ShiftVR ? Vec4f{ ShiftVR->x, -ShiftVR->y, ShiftVR->z, ShiftVR->w } : Vec4f{ 0.0f, 0.0f, 0.0f, 0.0f };
		quad[0].shift = quad[1].shift = quad[2].shift = quad[3].shift = shift;

		quad[0].lockToCamera = quad[1].lockToCamera = quad[2].lockToCamera = quad[3].lockToCamera = LockToCameraVR ? 1 : 0;

		quad[0].textureId_Color = textureId_Color;
		quad[1].textureId_Color = textureId_Color;
		quad[2].textureId_Color = textureId_Color;
		quad[3].textureId_Color = textureId_Color;
	}

	void screenGPU_drawColoredQuad(fixed16_16 x0, fixed16_16 y0, fixed16_16 x1, fixed16_16 y1, u8 color)
	{
		f32 fx0 = fixed16ToFloat(x0);
		f32 fy0 = fixed16ToFloat(y0);
		f32 fx1 = fixed16ToFloat(x1);
		f32 fy1 = fixed16ToFloat(y1);
		u32 textureId_Color = 0xffffu | (u32(color) << 16u) | (31u << 24u);

		ScreenQuadVertex* quad = &s_scrQuads[s_screenQuadCount * 4];
		s_screenQuadCount++;

		quad[0].posUv = { fx0, fy0, 0.0f, 1.0f };
		quad[1].posUv = { fx1, fy0, 1.0f, 1.0f };
		quad[2].posUv = { fx1, fy1, 1.0f, 0.0f };
		quad[3].posUv = { fx0, fy1, 0.0f, 0.0f };

		const Vec4f shift = ShiftVR ? Vec4f{ ShiftVR->x, -ShiftVR->y, ShiftVR->z, ShiftVR->w } : Vec4f{ 0.0f, 0.0f, 0.0f, 0.0f };
		quad[0].shift = quad[1].shift = quad[2].shift = quad[3].shift = shift;

		quad[0].lockToCamera = quad[1].lockToCamera = quad[2].lockToCamera = quad[3].lockToCamera = LockToCameraVR ? 1 : 0;

		quad[0].textureId_Color = textureId_Color;
		quad[1].textureId_Color = textureId_Color;
		quad[2].textureId_Color = textureId_Color;
		quad[3].textureId_Color = textureId_Color;
	}

	// Scaled versions.
	void screenGPU_blitTextureScaled(TextureData* texture, DrawRect* rect, fixed16_16 x0, fixed16_16 y0, fixed16_16 xScale, fixed16_16 yScale, u8 lightLevel, JBool forceTransparency)
	{
		if (s_screenQuadCount >= SCR_MAX_QUAD_COUNT)
		{
			return;
		}

		fixed16_16 x1 = x0 + mul16(intToFixed16(texture->width),  xScale);
		fixed16_16 y1 = y0 + mul16(intToFixed16(texture->height), yScale);
		s32 textureId = texture->textureId;

		u8 color = 0;
		u32 textureId_Color = textureId | (u32(color) << 16u) | (u32(lightLevel) << 24u);

		ScreenQuadVertex* quad = &s_scrQuads[s_screenQuadCount * 4];
		s_screenQuadCount++;

		f32 fx0 = fixed16ToFloat(x0);
		f32 fy0 = fixed16ToFloat(y0);
		f32 fx1 = fixed16ToFloat(x1);
		f32 fy1 = fixed16ToFloat(y1);

		////Mat4 cameraView = TFE_Math::buildMatrix4(s_cameraMtx, s_cameraPos);
		const Vec2f screenSize = { f32(s_scrQuadsWidth), f32(s_scrQuadsHeight) };
		//Mat4 cameraView = TFE_Math::getIdentityMatrix4();
		//Vec3f unprojected0 = TFE_Math::unprojectTo3D_({  100,  200 }, screenSize, s_cameraProj, cameraView, 1000.0f);
		//Vec3f unprojected1 = TFE_Math::unprojectTo3D_({  640,  360 }, screenSize, s_cameraProj, cameraView, 1000.0f);
		//Vec3f unprojected2 = TFE_Math::unprojectTo3D_({    0,    0 }, screenSize, s_cameraProj, cameraView, 1000.0f);
		//Vec3f unprojected3 = TFE_Math::unprojectTo3D_({ 1280,  720 }, screenSize, s_cameraProj, cameraView, 1000.0f);
		//Vec3f unprojected4 = TFE_Math::unprojectTo3D_({ 1280,    0 }, screenSize, s_cameraProj, cameraView, 1000.0f);
		//Vec3f unprojected5 = TFE_Math::unprojectTo3D_({    0,  720 }, screenSize, s_cameraProj, cameraView, 1000.0f);

		//Vec2f projected0 = TFE_Math::projectTo2D(unprojected0, screenSize, s_cameraProj, cameraView);
		//Vec2f projected1 = TFE_Math::projectTo2D(unprojected1, screenSize, s_cameraProj, cameraView);
		//Vec2f projected2 = TFE_Math::projectTo2D(unprojected2, screenSize, s_cameraProj, cameraView);
		//Vec2f projected3 = TFE_Math::projectTo2D(unprojected3, screenSize, s_cameraProj, cameraView);
		//Vec2f projected4 = TFE_Math::projectTo2D(unprojected4, screenSize, s_cameraProj, cameraView);
		//Vec2f projected5 = TFE_Math::projectTo2D(unprojected5, screenSize, s_cameraProj, cameraView);

		//float near, far, fov, aspect;
		//TFE_Math::decomposeProjectionMatrix(near, far, fov, aspect, s_cameraProj);

		f32 u0 = 0.0f, v0 = 0.0f;
		f32 u1 = f32(texture->width);
		f32 v1 = f32(texture->height);
		quad[0].posUv = { fx0, fy0, u0, v1 };
		quad[1].posUv = { fx1, fy0, u1, v1 };
		quad[2].posUv = { fx1, fy1, u1, v0 };
		quad[3].posUv = { fx0, fy1, u0, v0 };

		const Vec4f shift = ShiftVR ? Vec4f{ ShiftVR->x, -ShiftVR->y, ShiftVR->z, ShiftVR->w } : Vec4f{ 0.0f, 0.0f, 0.0f, 0.0f };
		quad[0].shift = quad[1].shift = quad[2].shift = quad[3].shift = shift;

		quad[0].lockToCamera = quad[1].lockToCamera = quad[2].lockToCamera = quad[3].lockToCamera = LockToCameraVR ? 1 : 0;

		quad[0].textureId_Color = textureId_Color;
		quad[1].textureId_Color = textureId_Color;
		quad[2].textureId_Color = textureId_Color;
		quad[3].textureId_Color = textureId_Color;
	}

	void screenGPU_blitTexture(ScreenImage* texture, DrawRect* rect, s32 x0, s32 y0)
	{
	}

	void screenGPU_blitTextureScaled(ScreenImage* texture, DrawRect* rect, s32 x0, s32 y0, fixed16_16 xScale, fixed16_16 yScale)
	{
	}

	void screenGPU_blitTextureLitScaled(ScreenImage* texture, DrawRect* rect, s32 x0, s32 y0, fixed16_16 xScale, fixed16_16 yScale, u8 lightLevel)
	{
	}

	void screenGPU_blitTextureIScale(TextureData* texture, DrawRect* rect, s32 x0, s32 y0, s32 scale)
	{
	}

}  // TFE_Jedi