
import time
import gravity
from gravity import GravityNode, GravityDataProduct, gravity

gn = GravityNode()
while gn.init("PyPub") != gravity.SUCCESS:
    print "failed to init, retrying..."
    time.sleep(1)
gn.registerDataProduct("PythonGDP", gravity.TCP)

gdp = GravityDataProduct("PythonGDP")
gdp.setData(bytearray("12345"))
gn.publish(gdp)

gn.waitForExit()
