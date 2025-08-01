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
import warnings
warnings.simplefilter(action='ignore', category=FutureWarning)  # quiet TensorFlow warnings

import time, sys
from gravity import GravityNode, GravityDataProduct, gravity, GravitySubscriber, SpdLogHandler
from DataPoint_pb2 import DataPointPB
from datetime import datetime
from gravity_autoencoder import TrainModel
from enum import Enum



training_size = 1000
training_data = {}
class TrainingState(Enum):
    COLLECTING=1
    TRAINING=2
    TRAINED=3
epochs = 3


class MySubscriber(GravitySubscriber):
    # We're only using GravitySubscriber here, but this is generally the way we 
    # initialize Gravity Python components to ensure that __init__ is called
    # on each parent.
    def __init__(self):
        GravitySubscriber.__init__(self)
        self.subs = 0
        self.train_state = TrainingState.COLLECTING

    def subscriptionFilled(self, dataProducts):
        if self.train_state != TrainingState.COLLECTING: return  # ignore
        pointPB = DataPointPB()
        for gdp in dataProducts:
            gdp.populateMessage(pointPB)
            if pointPB.name not in training_data:
                training_data[pointPB.name] = []
            training_data[pointPB.name].append((pointPB.timestamp, pointPB.value))
            self.subs += 1
            if self.subs == 100:
                self.subs = 0
                gravlogger.warning("Received %d of %d training samples. "%([len(v) for v in training_data.values()][0], training_size))
                #Log.message("Training Subscriptions: " + str([(k,len(v)) for k,v in training_data.items()]))
        
        has_enough_data = True
        for k,v in training_data.items():
            if len(v) < training_size: 
                has_enough_data = False
                break
        if has_enough_data:
            gravlogger.warning("Calling trainmodel")
            self.train_state = TrainingState.TRAINING
            TrainModel(training_data, model_file, epochs=epochs)
            self.train_state = TrainingState.TRAINED

gravlogger = logging.getLogger()
gravlogger.setLevel(logging.WARNING)  # let all logs pass through to Gravity logger
gravlogger.addHandler(SpdLogHandler(True))
mySub = MySubscriber()

gn = GravityNode()
while gn.init("AnomalyDetector") != gravity.SUCCESS:
    gravlogger.warning("failed to init, retrying...")
    time.sleep(1)

epochs = gn.getIntParam("training_epochs", epochs)
training_size = gn.getIntParam("training_size", training_size)
channel = gn.getStringParam("subscription_name", "channel")
model_file = gn.getStringParam("model_file", "model.json")
gn.subscribe(channel, mySub)

while mySub.train_state != TrainingState.TRAINED: time.sleep(1)
gravlogger.warning("Training Complete " + str(mySub.train_state))

gn.unsubscribe(channel, mySub)
sys.exit(0)
