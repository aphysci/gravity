#!/usr/bin/python
#** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
#**
#** Gravity is free software; you can redistribute it and/or modify
#** it under the terms of the GNU Lesser General Public License as published by
#** the Free Software Foundation; either version 3 of the License, or
#** (at your option) any later version.
#**
#** This program is distributed in the hope that it will be useful,
#** but WITHOUT ANY WARRANTY; without even the implied warranty of
#** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#** GNU Lesser General Public License for more details.
#**
#** You should have received a copy of the GNU Lesser General Public
#** License along with this program;
#** If not, see <http://www.gnu.org/licenses/>.
#**

import logging
import time,sys
from gravity import GravityNode, GravityDataProduct, gravity, GravitySubscriber, SpdLogHandler
from DataPoint_pb2 import DataPointPB
from datetime import datetime
import numpy as np


gravlogger = logging.getLogger()
gravlogger.setLevel(logging.WARNING)  # let all logs pass through to Gravity logger
gravlogger.addHandler(SpdLogHandler(True))
name = 'Pub'
anomaly_point = None
if len(sys.argv) > 1:
    anomaly_point = int(sys.argv[1])

gn = GravityNode()
while gn.init(name) != gravity.SUCCESS:
    Log.warning("failed to init, retrying...")
    time.sleep(1)

inc1 = gn.getFloatParam("increment1", 0.05)
inc2 = gn.getFloatParam("increment2", 0.05)
channel = gn.getStringParam("subscription_name", "channel")
gn.registerDataProduct(channel, gravity.TCP)

pointPB1 = DataPointPB()
pointPB1.name = "Pub1"
gdp1 = GravityDataProduct(channel)

pointPB2 = DataPointPB()
pointPB2.name = "Pub2"
gdp2 = GravityDataProduct(channel)

sample_num = 0
p1 = p2 = 0
while True:
    pointPB1.value = np.sin(p1)
    p1 += inc1
    pointPB1.timestamp = int(datetime.now().timestamp() * 1000000)
    gdp1.data=pointPB1
    gn.publish(gdp1)

    pointPB2.value = np.cos(p2)
    p2 += inc2
    pointPB2.timestamp = int(datetime.now().timestamp() * 1000000)
    gdp2.data=pointPB2
    gn.publish(gdp2)

    time.sleep(0.1)
    sample_num += 1
    if anomaly_point and sample_num == anomaly_point:
        gravlogger.warning("Anomaly point reached")
        inc2 *= 2


