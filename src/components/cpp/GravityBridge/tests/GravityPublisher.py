#!/usr/bin/env python
# -*- coding: utf-8 -*-
import time
import gravity
from gravity import GravityNode, GravityDataProduct, gravity, GravitySubscriber, Log

gn = GravityNode()
while gn.init("PyPublisher") != gravity.SUCCESS:
    Log.warning("failed to init, retrying...")
    time.sleep(1)
gn.registerDataProduct("SampleData", gravity.TCP)

gdp = GravityDataProduct("SampleData")
for i in range (1, 601):
    gdp.setData("TEST %d" % i)
    gn.publish(gdp)
    time.sleep(1)

gn.waitForExit()
