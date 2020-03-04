# Gravity Machine Learning Example
This example shows a Gravity anomaly detector that can detect changes in the relationship between data streams. It shows how one might use TensorFlow to implement macine learning in Gravity.

There are two scripts that run Gravity node clusters:

1. **training.sh** -- Runs a publisher node that publishes two parallel streams of floating-point numbers. It also runs a subscriber node that receives those streams and trains an autoencoder to recognize them.  Some training configuration parameters are set in Gravity.ini.  Run this script first and it will train the model and store the trained model in "model.json" and "model.json.h5"  Once the training is complete, stop the nodes with Ctrl-C or by closing the windows.
2. **detect.sh** -- Runs the a publisher node that is configured to change one of the streams (an 'anomoly') after a certain number of samples.  The anomoly point is configured by passing a command-line argument to the python node.  The subscriber node runs the streams through the autoencoder trained in step 1 and reports the mean squred error.  Notice that the MSE is low initiaily because the streams match the training streams.  When the anomoly starts, the MSE increases noticably.



