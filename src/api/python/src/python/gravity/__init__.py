# (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
#
# Gravity is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program;
# If not, see <http://www.gnu.org/licenses/>.
#
import os
if hasattr(os, "add_dll_directory"): # Python >=3.8 on Windows does not search using PATH
    bin_dir = os.path.join(os.path.abspath(os.path.dirname(__file__)), "..", "..", "bin")
    os.add_dll_directory(bin_dir)
    
from .gravity import GravityNode, GravitySubscriber, GravityRequestor, GravityServiceProvider, GravityHeartbeatListener, Log, Logger, SpdLog
from .GravityDataProduct import GravityDataProduct 
from .GravityLogHandler import GravityLogHandler
from .SpdLogHandler import SpdLogHandler

