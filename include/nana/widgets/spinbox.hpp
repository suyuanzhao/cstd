/**
 *	A Spin box widget
 *	Nana C++ Library(http://www.nanapro.org)
 *	Copyright(C) 2003-2017 Jinhao(cnjinhao@hotmail.com)
 *
 *	Distributed under the Boost Software License, Version 1.0.
 *	(See accompanying file LICENSE_1_0.txt or copy at
 *	http://www.boost.org/LICENSE_1_0.txt)
 *
 *	@file: nana/gui/widgets/spinbox.hpp
 */

#ifndef NANA_GUI_WIDGET_SPINBOX_HPP
#define NANA_GUI_WIDGET_SPINBOX_HPP


#include "widget.hpp"
#include "skeletons/text_editor_part.hpp"

namespace nana
{
	class spinbox;

	struct arg_spinbox
		: public event_arg
	{
		spinbox & widget;
		arg_spinbox(spinbox&);
	};

	namespace drawerbase
	{
		namespace spinbox
		{
			struct spinbox_events
				: public general_events
			{
				basic_event<arg_spinbox> text_changed;
			};

			/// Declaration of internal spinbox implementation
			class implementation;

			/// Drawer of spinbox
			class drawer
				: public drawer_trigger
			{
				drawer(const drawer&) {}
				drawer(drawer&) {}
				drawer& operator=(const drawer&) {}
				drawer& operator=(drawer&) {}
			public:
				drawer();
				~drawer();
				implementation * impl() const;
			private:
				//Overrides drawer_trigger
				void attached(widget_reference, graph_reference);
				void detached();
				void refresh(graph_reference)	override;

				void focus(graph_reference, const arg_focus&)	override;
				void mouse_wheel(graph_reference, const arg_wheel&);
				void dbl_click(graph_reference, const arg_mouse&);
				void mouse_down(graph_reference, const arg_mouse&)	override;
				void mouse_move(graph_reference, const arg_mouse&)	override;
				void mouse_up(graph_reference, const arg_mouse& arg)	override;
				void mouse_leave(graph_reference, const arg_mouse&)	override;
				void key_press(graph_reference, const arg_keyboard&);
				void key_char(graph_reference, const arg_keyboard&);
				void resized(graph_reference, const arg_resized&);
			private:
				implementation * const impl_;
			};
		}
	}//end namespace drawerbase

	/// Spinbox Widget
	class spinbox
		: public widget_object <category::widget_tag, drawerbase::spinbox::drawer, drawerbase::spinbox::spinbox_events, widgets::skeletons::text_editor_scheme>
	{
	public:
		/// Constructs a spinbox.
		spinbox();
		spinbox(window, bool visible);
		spinbox(window, const rectangle& = {}, bool visible = true);

		/// Sets the widget whether it accepts user keyboard input.
		/// @param accept Set to indicate whether it accepts uesr keyboard input.
		void editable(bool accept);

		/// Determines whether the widget accepts user keyboard input.
		bool editable() const;

		/// Sets the numeric spin values and step.
		void range(int begin, int last, int step);
		void range(double begin, double last, double step);

		/// Sets the string spin values.
		void range(vector<string> values_utf8);

		vector<string> range_string() const;
		pair<int, int> range_int() const;
		pair<double, double> range_double() const;

		/// Gets the spined value
		::string value() const;
		void value(const char*);
		int to_int() const;
		double to_double() const;

		/// Sets the modifiers
		void modifier(string prefix_utf8, string suffix_utf8);
		void modifier(const wstring & prefix, const wchar* suffix);
	private:
		native_string_type _m_caption() const;
		void _m_caption(native_string_type&&);
	}; //end class spinbox
}//end namespace nana

#endif //NANA_GUI_WIDGET_SPINBOX_HPP