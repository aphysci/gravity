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

import warnings
warnings.simplefilter(action='ignore', category=FutureWarning)  # quiet TensorFlow warnings

import time, sys
from gravity import GravityNode, GravityDataProduct, gravity, GravitySubscriber, Log
from DataPoint_pb2 import DataPointPB
from datetime import datetime
from gravity_autoencoder import GravityModel
from enum import Enum
from functools import reduce
from queue import Queue

# TensorFlow models are not thread-safe, so we communicate data from the 
# subscription callback thread to the main thread using a producer-
# consumer queue.
data = Queue()

class MySubscriber(GravitySubscriber):
    def __init__(self):
        GravitySubscriber.__init__(self)
        self.subs = 0
        self.sample_data = {}

    def subscriptionFilled(self, dataProducts):
        pointPB = DataPointPB()
        for gdp in dataProducts:
            gdp.populateMessage(pointPB)
            if pointPB.name not in self.sample_data:
                self.sample_data[pointPB.name] = []
            self.sample_data[pointPB.name].append((pointPB.timestamp, pointPB.value))
            # See if we have enough data from every stream to do a run of the autoencoder
            # A real example would probably need to time-align and possibly interpolate the data
            min_length = reduce(lambda a,b: min(a,len(self.sample_data[b])), self.sample_data, len(self.sample_data[list(self.sample_data)[0]]))
            if min_length >= model.n_steps:
                data.put(self.sample_data)
                self.sample_data = {}


gn = GravityNode()
while gn.init("AnomalyDetector") != gravity.SUCCESS:
    Log.warning("failed to init, retrying...")
    time.sleep(1)

model_filename = gn.getStringParam("model_file", "model.json")
model = GravityModel(model_filename)
mySub = MySubscriber()

channel = gn.getStringParam("subscription_name", "channel")
gn.subscribe(channel, mySub)

while True: 
    d = data.get()
    mse = model.ComputeMSE(d)
    Log.message("MSE = %f"%(mse))

# notreached
gn.unsubscribe(channel, mySub)
sys.exit(0)
