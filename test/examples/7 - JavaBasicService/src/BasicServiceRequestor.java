import Multiplication.MultiplicationOperandsPB;
import Multiplication.MultiplicationResultPB;

import com.aphysci.gravity.GravityDataProduct;
import com.aphysci.gravity.GravityRequestor;
import com.aphysci.gravity.swig.GravityNode;
import com.aphysci.gravity.swig.Log;
import com.aphysci.gravity.swig.Log.LogLevel;


//After multiplication is requested, this class may be called with the result.  
class MultiplicationRequestor implements GravityRequestor
{
	public void requestFilled(String serviceID, String requestID, GravityDataProduct response)
	{
		//Parse the message into a protobuf.  
		Multiplication.MultiplicationResultPB result;
		response.populateMessage(result);
		
		//Write the answer
		Log.message(String.format("%s: %d", requestID, result.getResult()));
		
		gotAsyncMessage = true;
	}

	boolean gotAsyncMessage = false;
	public boolean gotMessage() { return gotAsyncMessage; }
}


public class BasicServiceRequestor {

	
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		GravityNode gn;
		//Initialize gravity, giving this node a componentID.  
		gn.init("MultiplicationRequestor");
		
		// Tell the logger to also log to the console.  
		Log.initAndAddConsoleLogger(LogLevel.MESSAGE);	

		/////////////////////////////
		// Set up the first multiplication request
		MultiplicationRequestor requestor = new MultiplicationRequestor();
		
		GravityDataProduct multRequest1 = new GravityDataProduct("Multiplication");
		Multiplication.MultiplicationOperandsPB.Builder params1 = Multiplication.MultiplicationOperandsPB.newBuilder();
		params1.setMultiplicandA(8);
		params1.setMultiplicandB(2);
		multRequest1.setData(params1.build());
		
		// Make an Asyncronous request for multiplication
		gn.request("Multiplication", //Service Name
					multRequest1, //Request
					requestor, //Object containing callback that will get the result.  
					"8 x 2"); //A string that identifies which request this is.  

		/////////////////////////////////////////
		//Set up the second multiplication request
		GravityDataProduct multRequest2 = new GravityDataProduct("Multiplication");
		Multiplication.MultiplicationOperandsPB.Builder params2 = Multiplication.MultiplicationOperandsPB.newBuilder();
		params2.setMultiplicandA(5);
		params2.setMultiplicandB(7);
		multRequest2.setData(params2.build());
		
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
			Multiplication.MultiplicationResultPB result;
			multSync.populateMessage(result);
			
			Log.message(String.format("5 x 7 = %d", result.getResult()));
		}

		/////////////////////////////////////////
		//Wait for the Asynchronous message to come in.  
		while(!requestor.gotMessage())
		{
			Thread.sleep(1000);
		}
	}

}
