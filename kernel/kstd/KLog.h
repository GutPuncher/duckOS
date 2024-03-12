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

#pragma once

namespace KLog {
	void dbg(const char* component, const char* fmt, ...);
	void info(const char* component, const char* fmt, ...);
	void success(const char* component, const char* fmt, ...);
	void warn(const char* component, const char* fmt, ...);
	void err(const char* component, const char* fmt, ...);
	void crit(const char* component, const char* fmt, ...);

#define KLOG_CONDITIONAL(method) \
	template<bool C, typename... ArgTs> \
	constexpr void method##_if(const char* component, const char* fmt, ArgTs... args) { \
		if constexpr(C) \
			method(component, fmt, args...); \
	}

	KLOG_CONDITIONAL(dbg);
	KLOG_CONDITIONAL(info);
	KLOG_CONDITIONAL(success);
	KLOG_CONDITIONAL(warn);
	KLOG_CONDITIONAL(err);
	KLOG_CONDITIONAL(crit);
}
