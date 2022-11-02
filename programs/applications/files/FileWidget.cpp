/*
	This file is part of duckOS.

	duckOS is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	duckOS is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#include "FileWidget.h"
#include <libui/widget/Label.h>
#include <libui/widget/Image.h>
#include <libui/libui.h>

using namespace UI;

FileWidget::FileWidget(const Duck::DirectoryEntry& entry, Ptr<DirectoryWidget> dir_widget):
	BoxLayout(VERTICAL), entry(entry), dir_widget(std::move(dir_widget))
{
	Ptr<const Gfx::Image> image;
	auto path = entry.path();

	if(path.extension() == "icon" || path.extension() == "png")
		image = Gfx::Image::load(entry.path()).value_or(nullptr);

	if(!image) {
		auto app = App::app_for_file(path);
		if(app.has_value())
			image = app.value().icon();
		else
			image = UI::app_info().resource_image(entry.is_directory() ? "folder.png" : "file.png");
	}

	auto ui_image = UI::Image::make(image);
	ui_image->set_preferred_size({32, 32});

	add_child(ui_image);
	add_child(UI::Label::make(entry.name()));
}

bool FileWidget::on_mouse_button(Pond::MouseButtonEvent evt) {
	if((evt.old_buttons & POND_MOUSE1) && !(evt.new_buttons & POND_MOUSE1)) {
		if(App::open(entry.path()).is_error())
			dir_widget->set_directory(entry.path());
		return true;
	}
	return false;
}