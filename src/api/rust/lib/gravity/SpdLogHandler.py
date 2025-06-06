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

import logging
from logging import Handler

from .gravity import SpdLog

class SpdLogHandler(Handler):
    def __init__(self, splitlines=False):
        super(SpdLogHandler, self).__init__()
        self.__splitlines = splitlines

    def emit(self, record):
        logFunc = None
        if record.levelno == logging.DEBUG:
            logFunc = SpdLog.debug
        elif record.levelno == logging.INFO:
            logFunc = SpdLog.info
        elif record.levelno == logging.WARNING:
            logFunc = SpdLog.warn
        elif record.levelno == logging.ERROR:
            logFunc = SpdLog.error
        elif record.levelno == logging.CRITICAL:
            logFunc = SpdLog.critical
            
        logMessage = str(self.format(record))
        # Ensures we can get a full stack trace in gravity log
        if self.__splitlines:
            for line in logMessage.splitlines():
                logFunc(line)
        else:
            logFunc(logMessage)            
        
