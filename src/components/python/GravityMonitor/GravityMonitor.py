#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
A simple real-time Gravity publication monitor.
"""

import sys
import time
import Queue as queue
import datetime
import collections
from itertools import islice, izip

import gravity
from gravity import gravity as GRAVITY
from gravity.ServiceDirectoryMapPB_pb2 import ServiceDirectoryMapPB

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import GLib, Gtk

class GravityMonitorModel(Gtk.ListStore, gravity.GravitySubscriber, gravity.GravityRequestor):
    """ListStore containing four fields, and a dict with other metadata.

    Periodically polls the gravity ServiceDirectory for a list of publications, and subscribes
    to every known publication.  Column List:
       [0]: Data Product ID (str)
       [1]: Publisher Component ID (str)
       [2]: Publisher Domain (str)
       [3]: Protocol (str)
       [4]: Type Name (str)
       [5]: Last Publication Time (str / iso8601)
       [6]: Frequency (float / Hz)
       [7]: dictionary of other metadata (dict)
    """
    def __init__(self, gravity_node):
        # super() doesn't work, likely due to bugs in one or both C extensions...
        Gtk.ListStore.__init__(self, str, str, str, str, str, str, float, object)
        gravity.GravitySubscriber.__init__(self)
        gravity.GravityRequestor.__init__(self)

        self.node = gravity_node
        self.index = {}

        # FIFO Queue to pass data products into the GTK/Main thread from Gravity.
        self.queue = queue.Queue()

        self.update_registry()
        GLib.timeout_add_seconds(10, self.update_registry)
        GLib.timeout_add_seconds(1, self.update_frequency)

    def subscriptionFilled(self, data_products):
        """Gravity Subscription callback - can't touch model directly due to multithreading"""
        self.queue.put(('SUB', data_products))
        GLib.idle_add(self.handle_gravity_events)

    # pylint: disable=unused-argument
    def requestFilled(self, service_id, request_id, data_product):
        """Gravity Service Directory updated - can't touch model directly due to multithreading."""
        service_directory = ServiceDirectoryMapPB()
        data_product.populateMessage(service_directory)
        self.queue.put(('DIR', service_directory))
        GLib.idle_add(self.handle_gravity_events)

    def handle_gravity_events(self):
        """Handle any recently received publications."""
        while True:
            try:
                event_type, data = self.queue.get_nowait()

            except queue.Empty:
                return False

            if event_type == 'DIR':
                # pylint: disable=no-member
                for provider in data.data_provider:
                    if provider.product_id in self.index.keys():
                        continue

                    self.index[provider.product_id] = self[self.append([
                        provider.product_id, provider.component_id[0], provider.domain_id[0], "", "", "", 0,
                        {'history': collections.deque(maxlen=10)}
                    ])]

                    self.node.subscribe(provider.product_id.encode('utf-8'), self)

            elif event_type == 'SUB':
                for data_product in data:
                    row = self.index[data_product.dataProductID]
                    row[1] = data_product.componentID
                    row[2] = data_product.domain
                    row[3] = data_product.protocol
                    row[4] = data_product.typeName
                    stamp = data_product.timestamp / 1000000.0
                    row[5] = datetime.datetime.utcfromtimestamp(stamp).strftime("%Y%m%dT%H%M%S.%fZ")
                    # Let frequency get updated by the periodic callback, otherwise it is spastic
                    # for 10Hz+ data as it bounces all around.
                    row[7]['last_data'] = data_product.data
                    row[7]['history'].append(stamp)

    def update_frequency(self):
        """Update the estimate of publication frequency."""
        for row in self:
            history = row[7]['history']
            if len(history) < 2:
                row[6] = 0.0
                continue

            differences = [x-y for x, y in izip(islice(history, 1, None), islice(history, 0, None))]
            average_difference = sum(differences) / len(differences)

            # If it's been an abnormally long time (3x frequency...) then display zero.
            time_since_last = time.time() - history[-1]
            if time_since_last > 3 * average_difference:
                row[6] = 0.0
            else:
                row[6] = 1.0 / average_difference
        return True

    def update_registry(self):
        """Check for any new publications."""
        gdp = gravity.GravityDataProduct("GetProviders")
        self.node.request("DirectoryService", gdp, self, "")
        return True # Or periodic updates won't work.

class GravityMonitorView(Gtk.TreeView):
    """A GTK TreeView to monitor Gravity publications."""
    def __init__(self, model):
        super(GravityMonitorView, self).__init__(model)
        self.model = model

        renderer = Gtk.CellRendererText()

        self.add_column("Data Product", renderer, 0)
        self.add_column("Publisher", renderer, 1)
        self.add_column("Domain", renderer, 2)
        self.add_column("Protocol", renderer, 3)
        self.add_column("Data Type", renderer, 4)
        self.add_column("Last Time", renderer, 5)
        col = self.add_column("Frequency", renderer, 6)
        fmt_float = lambda c, cell, model, it, X: cell.set_property('text', "%0.1f" % model[it][6])
        col.set_cell_data_func(renderer, fmt_float)

    def add_column(self, field_name, renderer, model_column):
        """Add a column for a given field, setting options appropriately."""
        column = Gtk.TreeViewColumn(field_name, renderer, text=model_column)
        column.set_sort_column_id(model_column)
        column.set_resizable(True)
        self.append_column(column)
        return column

class GravityMonitorWindow(Gtk.Window):
    """Window for monitoring Gravity publications in realtime."""
    def __init__(self):
        super(GravityMonitorWindow, self).__init__(title="Gravity Monitor")
        self.set_wmclass ("Gravity Monitor", "Gravity Monitor")
        icon = Gtk.IconTheme.get_default().load_icon("utilities-system-monitor", 128, 0)
        self.set_icon(icon)

        self.node = gravity.GravityNode()

        attempts = 0
        while self.node.init("GravityMonitor") != GRAVITY.SUCCESS and attempts < 5:
            print "Failed to initialize Gravity, retrying..."
            time.sleep(0.5)
            attempts += 1

        if attempts == 5:
            print "Unable to initialize Gravity, quitting."
            sys.exit(1)

        self.store = GravityMonitorModel(self.node)
        self.treeview = GravityMonitorView(self.store)
        self.add(self.treeview)

def main():
    """Create and run a single GravityMonitorWindow."""
    gmw = GravityMonitorWindow()
    gmw.connect("destroy", Gtk.main_quit)
    gmw.show_all()
    Gtk.main()

if __name__ == "__main__":
    main()
