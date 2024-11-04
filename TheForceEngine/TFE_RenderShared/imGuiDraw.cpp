#include "ImGuiDraw.h"
#include <TFE_RenderBackend/renderBackend.h>
#include <TFE_RenderBackend/shader.h>
#include <TFE_RenderBackend/vertexBuffer.h>
#include <TFE_RenderBackend/indexBuffer.h>
#include <TFE_System/system.h>
#include <TFE_Settings/settings.h>
#include <TFE_System/math.h>
#include <TFE_Vr/vr.h>
#include <TFE_Ui/imGUI/imgui.h>

namespace TFE_RenderBackend
{
	extern Mat4  s_cameraProjVR[2];
	extern Mat4  s_cameraProjVR_YDown[2];
	extern Mat3  s_cameraMtxVR[2];
	extern Mat3  s_cameraMtxVR_YDown[2];
	extern Vec3f s_cameraPosVR[2];
	extern Vec3f s_cameraPosVR_YDown[2];
}

namespace TFE_RenderShared
{
	// Vertex Definition
	struct QuadVertex
	{
		Vec2f pos;		// 2D position.
		Vec2f uv;		// UV coordinates.
		u32   color;	// color + opacity.
	};
	static const AttributeMapping c_quadAttrMapping[]=
	{
		{ATTR_POS,   ATYPE_FLOAT, 2, 0, false},
		{ATTR_UV,    ATYPE_FLOAT, 2, 0, false},
		{ATTR_COLOR, ATYPE_UINT8, 4, 0, true}
	};
	static const u32 c_quadAttrCount = TFE_ARRAYSIZE(c_quadAttrMapping);

	//static s32 s_svScaleOffsetId = -1;
	//static s32 s_cameraPosId = -1;
	//static s32 s_cameraViewId = -1;
	static s32 s_cameraProjId = -1;
	static s32 s_screenSizePosId = -1;
	static s32 s_frustumId = -1;
	static s32 s_HmdViewId = -1;
	static s32 s_mousePosId = -1;
	static s32 s_dotSizeId = -1;
	static s32 s_dotColorId = -1;
	static s32 s_ShiftId = -1;
	static s32 s_LockToCameraId = -1;
	static s32 s_ClipRectId = -1;
	//static const ShaderUniform* s_cameraPosUniforms{ nullptr };
	//static const ShaderUniform* s_cameraViewUniforms{ nullptr };
	//static const ShaderUniform* s_cameraProjUniforms{ nullptr };

	static Shader s_shader;
	static VertexBuffer s_vertexBuffer;
	static IndexBuffer  s_indexBuffer;
	static TextureGpu* s_fontTexture{ nullptr };

	bool imGuiDraw_init()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.BackendRendererName = "TFE_imgui_renderer";

		u32 defineCount = 0;
		ShaderDefine defines[2];
		if (TFE_Settings::getTempSettings()->vr)
		{
			defines[defineCount++] = { "OPT_VR", "1" };
			if (TFE_Settings::getTempSettings()->vrMultiview)
			{
				defines[defineCount++] = { "OPT_VR_MULTIVIEW", "1" };
			}
		}

		if (!s_shader.load("Shaders/imGui.vert", "Shaders/imGui.frag", defineCount, defines, SHADER_VER_STD))
		{
			return false;
		}
		s_shader.bindTextureNameToSlot("Image", 0);

		// VR specific
		//s_cameraPosId = s_shader.getVariableId("CameraPos", &s_cameraPosUniforms);
		//s_cameraViewId = s_shader.getVariableId("CameraView", &s_cameraViewUniforms);
		s_cameraProjId = s_shader.getVariableId("CameraProj"/*, &s_cameraProjUniforms*/);
		s_screenSizePosId = s_shader.getVariableId("ScreenSize");
		s_frustumId = s_shader.getVariableId("Frustum");
		s_HmdViewId = s_shader.getVariableId("HmdView");
		s_mousePosId = s_shader.getVariableId("MousePos");
		s_dotSizeId = s_shader.getVariableId("DotSize");
		s_dotColorId = s_shader.getVariableId("DotColor");
		s_ShiftId = s_shader.getVariableId("Shift");
		s_LockToCameraId = s_shader.getVariableId("LockToCamera");
		s_ClipRectId = s_shader.getVariableId("ClipRect");

		//s_svScaleOffsetId = s_shader.getVariableId("ScaleOffset");
		//if (s_svScaleOffsetId < 0)
		//{
		//	return false;
		//}

		s_vertexBuffer.create(0 * 128 * 1024, sizeof(QuadVertex), c_quadAttrCount, c_quadAttrMapping, true);
		s_indexBuffer.create(0 * 128 * 1024, sizeof(ImDrawIdx), true);

		return true;
	}

	void imGuiDraw_destroy()
	{
		s_vertexBuffer.destroy();
		s_indexBuffer.destroy();

		imGuiDraw_destroyFontTexture();
	}

	void imGuiDraw_createFontTexture()
	{
		//return;
		if (s_fontTexture) { return; }

		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		s_fontTexture = new TextureGpu{};
		s_fontTexture->createWithData(width, height, pixels, MAG_FILTER_LINEAR);
		io.Fonts->TexID = (ImTextureID)(intptr_t)s_fontTexture->getHandle();
	}

	void imGuiDraw_destroyFontTexture()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->TexID = 0;

		delete s_fontTexture;
		s_fontTexture = nullptr;
	}

	void imGuiDraw_render()
	{
		ImDrawData* draw_data = ImGui::GetDrawData();
		if (!draw_data || draw_data->CmdListsCount < 1)
		{
			return;
		}

		TFE_RenderState::setStateEnable(false, STATE_CULLING | STATE_DEPTH_TEST | STATE_DEPTH_WRITE);
		TFE_RenderState::setStateEnable(true, STATE_BLEND);
		TFE_RenderState::setBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		TFE_RenderBackend::setScissorRect(false); // handled in fragment shader

		s_shader.bind();

		if (TFE_Settings::getTempSettings()->vr)
		{
			if (!TFE_Settings::getTempSettings()->vrMultiview)
			{
				TFE_ERROR("VR", "non multiview not handled yet");
			}

			const std::array<Vec3f, 8>& frustum = vr::GetUnitedFrustum();
			Mat3 hmdMtx = TFE_Math::getMatrix3(vr::GetEyePose(vr::Side::Left).mTransformation);

			const Vec2ui& targetSize = vr::GetRenderTargetSize();
			const ImGuiIO& io = ImGui::GetIO();
			const TFE_Settings_Vr* vrSettings = TFE_Settings::getVrSettings();

			s_shader.setVariableArray(s_cameraProjId, SVT_MAT4x4, TFE_RenderBackend::s_cameraProjVR[0].data, 2);
			//	s_shader.setVariable(s_cameraProjId, SVT_MAT4x4, s_cameraProj.data);
			s_shader.setVariable(s_screenSizePosId, SVT_VEC2, Vec2f{ f32(targetSize.x), f32(targetSize.y) }.m);
			s_shader.setVariableArray(s_frustumId, SVT_VEC3, frustum.data()->m, (u32)frustum.size());
			s_shader.setVariable(s_HmdViewId, SVT_MAT3x3, hmdMtx.data);
			s_shader.setVariable(s_mousePosId, SVT_VEC2, Vec2f{ f32(io.MousePos.x), f32(io.MousePos.y) }.m);
			s_shader.setVariable(s_dotSizeId, SVT_SCALAR, &vrSettings->configDotSize);
			const RGBA& col = vrSettings->configDotColor;
			s_shader.setVariable(s_dotColorId, SVT_VEC4, Vec4f{ col.getRedF(), col.getGreenF(), col.getBlueF(), col.getAlphaF() }.m);
			const Vec3f& shift = vrSettings->configToVr.shift;
			s_shader.setVariable(s_ShiftId, SVT_VEC4, Vec4f{ shift.x, shift.y, shift.z, vrSettings->configToVr.distance }.m);
			s32 lock = vrSettings->configToVr.lockToCamera ? 1 : 0;
			s_shader.setVariable(s_LockToCameraId, SVT_ISCALAR, &lock);
		}
		else
		{
			float L = draw_data->DisplayPos.x;
			float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
			float T = draw_data->DisplayPos.y;
			float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
			const float ortho_projection[4][4] =
			{
				{ 2.0f / (R - L),0.0f,		     0.0f,   (R + L) / (L - R) },
				{ 0.0f,         2.0f / (T - B),  0.0f,   (T + B) / (B - T) },
				{ 0.0f,         0.0f,			-1.0f,   0.0f },
				{ 0.0f,			0.0f,			 0.0f,	 1.0f },
			};
			s_shader.setVariable(s_cameraProjId, SVT_MAT4x4, (const float*)ortho_projection);
		}

		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
		int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
		if (fb_width <= 0 || fb_height <= 0)
			return;

		// Will project scissor/clipping rectangles into framebuffer space
		ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
		ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Render command lists
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];

			// Upload vertex/index buffers
			s_vertexBuffer.update(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			s_indexBuffer.update(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback != NULL)
				{
					TFE_ERROR("UI", "user callback not handled");
					// User callback, registered via ImDrawList::AddCallback()
					// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
					//if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					//	ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
					//else
					//	pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					// Project scissor/clipping rectangles into framebuffer space
					ImVec4 clip_rect;
					clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
					clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
					clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
					clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

					if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
					{
						// Apply scissor/clipping rectangle
						s_shader.setVariable(s_ClipRectId, SVT_VEC4, (const float*)&clip_rect.x);

						s_vertexBuffer.bind();
						s_indexBuffer.bind();
						TFE_RenderBackend::bindNativeTexture(pcmd->TextureId);

						TFE_RenderBackend::drawIndexedTriangles(pcmd->ElemCount / 3, sizeof(ImDrawIdx), pcmd->IdxOffset);
						// TODO: have to unbind the texture here, otherwise font texture (& probably others too) filtering is updated to GL_NEAREST somewhere, but only in VR mode &
						// when postprocess effects are active (bloom), it's affected by this two lines from postprocess.cpp
						// if (effectInst->forceLinearFilter) { effectInst->inputs[i].tex->setFilter(MAG_FILTER_LINEAR, MIN_FILTER_LINEAR); }
						// if (effectInst->forceLinearFilter) { effectInst->inputs[i].tex->setFilter(MAG_FILTER_NONE, MIN_FILTER_NONE); }
						// is it a driver bug or did I miss something?
						TFE_RenderBackend::bindNativeTexture(0); 
					}
				}
			}
		}

		// Cleanup.
		s_shader.unbind();
		s_vertexBuffer.unbind();
		s_indexBuffer.unbind();
	}
}
