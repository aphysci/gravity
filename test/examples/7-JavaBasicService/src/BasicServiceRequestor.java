/** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
 **
 ** Gravity is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation; either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this program;
 ** If not, see <http://www.gnu.org/licenses/>.
 **
 */

import com.aphysci.gravity.GravityDataProduct;
import com.aphysci.gravity.GravityRequestor;
import com.aphysci.gravity.swig.GravityNode;
import com.aphysci.gravity.swig.GravityReturnCode;
import com.aphysci.gravity.swig.Log;


//After multiplication is requested, this class may be called with the result.
class MultiplicationRequestor implements GravityRequestor
{
	public void requestFilled(String serviceID, String requestID, GravityDataProduct response)
	{
		//Parse the message into a protobuf.
		Multiplication.MultiplicationResultPB.Builder result = Multiplication.MultiplicationResultPB.newBuilder();
		response.populateMessage(result);

		//Write the answer
		Log.warning(String.format("%s: %d", requestID, result.getResult()));

		gotAsyncMessage = true;
	}

	boolean gotAsyncMessage = false;
	public boolean gotMessage() { return gotAsyncMessage; }
}


public class BasicServiceRequestor {



	/**
	 * @param args
	 * @throws InterruptedException
	 */
	public static void main(String[] args) throws InterruptedException {
		GravityNode gn = new GravityNode();
		//Initialize gravity, giving this node a componentID.
		GravityReturnCode ret = gn.init("MultiplicationRequestor");
		while (ret != GravityReturnCode.SUCCESS) {
			Log.warning("Unable to initialize component, retrying...");
			ret = gn.init("MultiplicationRequestor");
		}

		/////////////////////////////
		// Set up the first multiplication request
		MultiplicationRequestor requestor = new MultiplicationRequestor();

		GravityDataProduct multRequest1 = new GravityDataProduct("Multiplication");
		Multiplication.MultiplicationOperandsPB.Builder params1 = Multiplication.MultiplicationOperandsPB.newBuilder();
		params1.setMultiplicandA(8);
		params1.setMultiplicandB(2);
		multRequest1.setData(params1);

		// Make an Asynchronous request for multiplication
		do {
			ret = gn.request("Multiplication", //Service Name
					    	 multRequest1, //Request
						     requestor, //Object containing callback that will get the result.
						     "8 x 2"); //A string that identifies which request this is.
			// Service may not be registered yet
			if (ret != GravityReturnCode.SUCCESS)
			{
				Log.warning("Failed request to Multiplication, retrying...");
				Thread.sleep(1000);
			}
		} while (ret != GravityReturnCode.SUCCESS);

		/////////////////////////////////////////
		//Set up the second multiplication request
		GravityDataProduct multRequest2 = new GravityDataProduct("Multiplication");
		Multiplication.MultiplicationOperandsPB.Builder params2 = Multiplication.MultiplicationOperandsPB.newBuilder();
		params2.setMultiplicandA(5);
		params2.setMultiplicandB(7);
		multRequest2.setData(params2);

		//Make a Synchronous request for multiplication
		GravityDataProduct multSync = gn.request("Multiplication", //Service Name
															multRequest2, //Request
															1000); //Timeout in milliseconds
		if(multSync == null)
		{
			Log.critical("Request Returned NULL!");
		}
		else
		{
			Multiplication.MultiplicationResultPB.Builder result = Multiplication.MultiplicationResultPB.newBuilder();
			multSync.populateMessage(result);

			Log.warning(String.format("5 x 7 = %d", result.getResult()));
		}

		/////////////////////////////////////////
		//Wait for the Asynchronous message to come in.
		while(!requestor.gotMessage())
		{
			Thread.sleep(1000);
		}
	}

}
