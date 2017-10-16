
import time
import gravity
from gravity import GravityNode, GravityDataProduct, GravitySubscriber

gn = GravityNode()
while gn.init("PyPub") != gravity.SUCCESS:
    print "failed to init, retrying..."
    time.sleep(1)
gn.registerDataProduct("PythonPub", gravity.TCP)

gdp = GravityDataProduct("PythonPub")

class MySub(GravitySubscriber):
    def __init__(self):
        pass

gs = MySub()

gn.waitForExit()
