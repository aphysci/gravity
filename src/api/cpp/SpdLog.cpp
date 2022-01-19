/** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
 **
 ** Gravity is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation; either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this program;
 ** If not, see <http://www.gnu.org/licenses/>.
 **
 */

#include "SpdLog.h"

using namespace gravity;

void SpdLog::critical(const char* message)
{
	spdlog::critical(message);
}

void SpdLog::error(const char* message)
{
	spdlog::error(message);
}

void SpdLog::warn(const char* message)
{
	spdlog::warn(message);
}

void SpdLog::info(const char* message)
{
	spdlog::info(message);
}

void SpdLog::debug(const char* message)
{
	spdlog::debug(message);
}

void SpdLog::trace(const char* message)
{
	spdlog::trace(message);
}
