#include "Settings.h"
#include "Menu.h"
#include <iostream>
#include "Globals.h"

void Menu::RenderMenu(nk_context* ctx, ID3D11DeviceContext* d3dctx)
{
	if (nk_begin(ctx, "OverFlow", nk_rect(10, (Globals::g_iWindowHeight / 2) - 180, 180, 300), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
	{
		nk_layout_row_dynamic(ctx, 30, 1);
		if (nk_button_label(ctx, "Shutdown"))
			g_Settings::bShutDown = true;
	}

	nk_end(ctx);
	nk_d3d11_render(d3dctx, NK_ANTI_ALIASING_OFF);

	nk_input_begin(ctx);
	nk_input_end(ctx);
} 

{
	namespace ext
	{
		bool components::button(const std::string_view text) {
			return ImGui::Button(text.data());
		}

		void components::button(const std::string_view text, std::function<void()> cb) {
			if (components::button(text)) {
				cb();
			}
		}
	}

}

{
	skip_if_not_implemented();
	namespace ext
	{
		bool components::feature_checkbox(const std::string& text, program::features feature, bool& value)
		{
			if (ImGui::Checkbox(text.c_str(), &value))
			{
				g_script_loader->set_feature(feature, value);
				return true;
			}

			return false;
		}
	}

}

{
	roundline(ctx, nk_rect(10, (Globals::g_iWindowHeight / 2) - 180, 180, 300), nk_rgb(0, 0, 0), 1.0f);
	nk_layout_row_dynamic(ctx, 30, 1);
	if (nk_button_label(ctx, "Shutdown"))
		g_Settings::bShutDown = true;
		
}
delete <<
