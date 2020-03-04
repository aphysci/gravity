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

import math
import json
import os
from functools import reduce
from numpy import array
import numpy as np
import tensorflow as tf
from tensorflow.keras.models import Sequential,model_from_json
from tensorflow.keras.layers import LSTM, Dense, RepeatVector, TimeDistributed
from tensorflow.keras.callbacks import ModelCheckpoint, TensorBoard
from tensorflow.keras import optimizers

import matplotlib.pyplot as plt

n_steps = 20 # number of samples to autoencode
lr = 0.0001  # Adam optimizer parameter

# split a multivariate sequence into samples
def split_sequences(sequences, n_steps):
    X, y = list(), list()
    for i in range(len(sequences)):
        end_ix = i + n_steps
        if end_ix > len(sequences):
            break
        seq_x, seq_y = sequences[i:end_ix, :], sequences[end_ix-1, :]
        X.append(seq_x)
        y.append(seq_y)
    return array(X), array(y)

def TrainModel(training_data, model_filename, epochs=3):
    print("Training on %d sequences"%(len(training_data)))
    seqs = []
    min_length = reduce(lambda a,b: min(a,len(training_data[b])), training_data, len(training_data[list(training_data)[0]]))
    print("length is %d"%(min_length))
    for k in sorted(training_data):
        t = training_data[k][:min_length]  # truncate to the length of the shortest array
        t_vals = [x[1] for x in t]  # discard the timestamps (for now)
        t_seq = np.array(t_vals)
        t_seq = t_seq.reshape(len(t_seq), 1)
        seqs.append(t_seq)

    dataset = np.hstack(seqs)
    n_features = len(seqs)

    X, y = split_sequences(dataset, n_steps)
    print(X.shape, y.shape)

    lstm_autoencoder = Sequential()
    # Encoder
    lstm_autoencoder.add(LSTM(32, activation='relu', input_shape=(n_steps, n_features), return_sequences=True))
    lstm_autoencoder.add(LSTM(16, activation='relu', return_sequences=False))
    lstm_autoencoder.add(RepeatVector(n_steps))
    # Decoder
    lstm_autoencoder.add(LSTM(16, activation='relu', return_sequences=True))
    lstm_autoencoder.add(LSTM(32, activation='relu', return_sequences=True))
    lstm_autoencoder.add(TimeDistributed(Dense(n_features)))

    
    adam = tf.keras.optimizers.Adam(lr)
    lstm_autoencoder.compile(optimizer="adam", loss='mse')
    
    lstm_autoencoder_history = lstm_autoencoder.fit(X, X, 
                                                epochs=epochs, 
                                                batch_size=64, 
                                                verbose=1).history
    
  
    model_json = lstm_autoencoder.to_json()
    with open(model_filename, "w") as json_file:
        json_file.write(model_json)
    lstm_autoencoder.save_weights(model_filename + ".h5")
    print("Saved model to disk")

class GravityModel():
    def __init__(self, model_filename):
        print("Loading model from disk")
        json_file = open(model_filename, 'r')
        loaded_model_json = json_file.read()
        json_file.close()
        self.lstm_autoencoder = model_from_json(loaded_model_json)
        # load weights into new model
        self.lstm_autoencoder.load_weights(model_filename + ".h5")
        print("Loaded model from disk" + str(self.lstm_autoencoder))
        self.n_steps = n_steps

    def Flatten(self, X):
        flattened_X = np.empty((X.shape[0], X.shape[2]))  # sample x features array.
        for i in range(X.shape[0]):
            flattened_X[i] = X[i, (X.shape[1]-1), :]
        return(flattened_X)

    def ComputeMSE(self, data):
        seqs = []
        min_length = reduce(lambda a,b: min(a,len(data[b])), data, len(data[list(data)[0]]))
        if min_length < self.n_steps:
            raise ValueError
        for k in sorted(data):
            t = data[k][:min_length]
            t_vals = [x[1] for x in t]  # discard the timestamps (for now)
            t_seq = np.array(t_vals)
            t_seq = t_seq.reshape(len(t_seq), 1)
            seqs.append(t_seq)
        dataset = np.hstack(seqs)
        X, y = split_sequences(dataset, self.n_steps)
        predictions = self.lstm_autoencoder.predict(X, verbose=1)
        mse = np.mean(np.power(self.Flatten(X) - self.Flatten(predictions), 2), axis=1)
        return mse


# Test the autoencoder separately from Gravity
if __name__ == "__main__":
    fs = 10000
    secs = 5
    good_pct = 0.99
    train_pct = 0.80
    t1 = np.linspace(0, 200*np.pi, num=fs*secs)
    s1 = np.sin(t1)
    t2= t1
    split = int(len(t1) * good_pct)
    rate = np.linspace(1, 1.3, num = len(t1) - split)
    t2[split:] = t2[split:] * rate
    s2 = np.cos(t2)
    data = {}
    data["s1"] = list(zip(t1, s1))
    data["s2"] = list(zip(t1, s2))
    TrainModel(data, "model.json")
    gm = GravityModel("model.json")
    for k in list(data):
        data[k] = data[k][:n_steps]
    print(gm.ComputeMSE(data))

