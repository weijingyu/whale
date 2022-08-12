#pragma once

#include "core/layer.h"

#include "events/application_event.h"
#include "events/key_event.h"
#include "events/mouse_event.h"

namespace Whale {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetDarkThemeColors();
	private:
		bool m_BlockEvents = true;
	};

}
