
# Gravity Machine Learning Example
This example shows a Gravity anomaly detector that can detect changes in the relationship between data streams. It shows how one might use TensorFlow to implement machine learning in Gravity.

There are two scripts that run Gravity node clusters*:

If you don't already have TensorFlow installed, you should be able to install it by running:

`python -m pip install tensorflow`

## **train.sh** 
Runs a publisher node that publishes two parallel streams of floating-point numbers. It also runs a subscriber node that receives those streams and trains an autoencoder to recognize them.  Some training configuration parameters are set in Gravity.ini.  Run this script first and it will train the model and store the trained model in "model.json" and "model.json.h5"  The number of points used for training is specified by the parameter `training_size` in Gravity.ini.  The default value of 2000 should take a few minutes to collect.  After the samples are collected, the model is trained with the training vectors.  Model.fit function is called in verbose mode, so you can see it reduce the loss over the 10 training epochs.  Once the training is complete and the model is saved to disk, you will see the the message:

`Training Complete TrainingState.TRAINED`

Stop the nodes with Ctrl-C or by closing the windows.


## **detect.sh** 
Runs the a publisher node that is configured to change one of the streams (an 'anomaly') after a certain number of samples.  The anomaly point is configured by passing a command-line argument to the python node.  The subscriber node runs the streams through the autoencoder trained in step 1 and reports the mean squared error.  Notice that the MSE is low initially because the streams match the training streams. Watch the output of the publisher node to see when it starts inserting the anomaly.  When the anomaly starts after a few seconds, the MSE increases noticeably.

This code was last tested with Python 3.6.9 and TensorFlow version 2.1.0

*: The TensorFlow libraries may print warning messages to the terminal about CUDA support when you run these scripts.  The examples will work with or without CUDA so you can ignore these messages. For help configuring CUDA, here's a good place to start: https://www.tensorflow.org/install/gpu
