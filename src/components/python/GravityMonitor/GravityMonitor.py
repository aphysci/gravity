#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import time
import uuid
import datetime

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk
from gi.repository import GLib

import gravity
from gravity import gravity as GRAVITY
from gravity.ServiceDirectoryMapPB_pb2 import ServiceDirectoryMapPB

# Something like this needs to be in python bindings, this is obnoxious.
class FunctionWrappingSubscriber(gravity.GravitySubscriber):
  def __init__(self, function, *args, **kwargs):
    self.function = function
    self.args = args
    self.kwargs = kwargs
    super(FunctionWrappingSubscriber, self).__init__()

  def subscriptionFilled(self, dataProducts):
    self.function(dataProducts, *self.args, **self.kwargs)

#### BUG: Today, if a Subscriber goes out of scope, it will cause segfaults.

class GravityMonitorWindow(Gtk.Window):
    def __init__(self):
        super(GravityMonitorWindow, self).__init__(title="Gravity Monitor")

        self.node = gravity.GravityNode()
        self.subs = []# Just until bug #21 is fixed...

        attempts = 0
        while self.node.init("GravityMonitor") != GRAVITY.SUCCESS and attempts < 5:
            print "Failed to initialize Gravity, retrying..."
            time.sleep(0.5)
            attempts += 1

        if attempts == 5:
            print "Unable to initialize Gravity, quitting."
            sys.exit(1)

        #product_id, last_publisher, last_time_fmted, last_time, hertz
        self.store = Gtk.ListStore(str, str, str, float, float)
        self.store_index = {}
        self.treeview = Gtk.TreeView(self.store)
        self.add(self.treeview)

        renderer = Gtk.CellRendererText()
        for colnum, field in [
          (0, "Data Product"),
          (1, "Publisher"),
          (2, "Last Time"), # Skipping 3 b/c it's last time as a float.
          (4, "Frequency")]:
            column = Gtk.TreeViewColumn(field, renderer, text=colnum)
            column.set_sort_column_id(colnum)
            column.set_resizable(True)
            self.treeview.append_column(column)

        self.update_registry()

        GLib.timeout_add_seconds(10, self.update_registry)        

    def data_received(self, data_products):
        for data_product in data_products:
            idx = self.store_index[data_product.getDataProductID()]
            self.store[idx][1] = data_product.getComponentID()
            stamp = data_product.getGravityTimestamp() / 1000000.0
            last_stamp = self.store[idx][3]
            self.store[idx][2] = datetime.datetime.utcfromtimestamp(stamp).strftime("%Y%m%dT%H%M%S.%fZ")
            self.store[idx][3] = stamp
            self.store[idx][4] = 1.0 / (stamp - last_stamp)

    def update_registry(self):
        gdp = gravity.GravityDataProduct("GetProviders")
        response = self.node.request("DirectoryService", gdp)
        serviceDirectory = ServiceDirectoryMapPB()
        response.populateMessage(serviceDirectory)

        for provider in serviceDirectory.data_provider:
            if provider.product_id in self.store_index.keys():
              continue

            sub = FunctionWrappingSubscriber(self.data_received)
            self.subs.append(sub)
            self.node.subscribe(provider.product_id.encode('utf-8'), sub)

            self.store_index[provider.product_id] = self.store.append([
              provider.product_id, provider.component_id[0], "", 0, 0
            ])
        return True # Or periodic updates won't work.

def main(argv):
    gmw = GravityMonitorWindow()
    gmw.connect("destroy", Gtk.main_quit)
    gmw.show_all()
    Gtk.main()

if __name__ == "__main__":
    main(sys.argv)
