import com.aphysci.gravity.GravityDataProduct;
import com.aphysci.gravity.GravityServiceProvider;
import com.aphysci.gravity.swig.GravityNode;
import com.aphysci.gravity.swig.Log;
import com.aphysci.gravity.swig.Log.LogLevel;

class MultiplicationServiceProvider implements GravityServiceProvider
{
	public  MultiplicationServiceProvider()
	{
	}
	
	@Override
	public GravityDataProduct request(GravityDataProduct dataProduct)
	{
		//Just to be safe.  In theory this can never happen unless this class is registered with more than one serviceID types.  
		if(!dataProduct.getDataProductID().equals("Multiplication")) {
			Log.critical(String.format("Request is not for %s, not Multiplication!", dataProduct.getDataProductID()));
			return new GravityDataProduct("BadRequest");
		}

		//Get the parameters for this request.  
		Multiplication.MultiplicationOperandsPB.Builder params = Multiplication.MultiplicationOperandsPB.newBuilder();
		dataProduct.populateMessage(params);

		Log.message(String.format("%d x %d", params.getMultiplicandA(), params.getMultiplicandB()));
		
		//Do the calculation
		int result = params.getMultiplicandA() * params.getMultiplicandB();
		
		//Return the results to the requestor
		Multiplication.MultiplicationResultPB.Builder resultPB = Multiplication.MultiplicationResultPB.newBuilder();
		resultPB.setResult(result);
		
		GravityDataProduct resultDP = new GravityDataProduct("MultiplicationResult");
		resultDP.setData(resultPB.build());

		return resultDP;
	}
}


public class BasicServiceProvider {
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		GravityNode gn = new GravityNode();
		//Initialize gravity, giving this node a componentID.  
		gn.init("MultiplicationComponent");

		//Tell the logger to also log to the console.  
		Log.initAndAddConsoleLogger(LogLevel.MESSAGE);	
		
		MultiplicationServiceProvider msp = new MultiplicationServiceProvider();
		gn.registerService(
							//This identifies the Service to the service directory so that others can 
							// make a request to it.  
							"Multiplication", 
							//This assigns a port on this computer to the data product.  No need to remember 
							//this because the service directory will tell this to other components looking 
							//for this data product.  Simply assign a port number between 1024 and 65535 that 
							//is not in use on this machine.  	
							(short) 54534, 
							//Assign a transport type to the socket (almost always tcp, unless you are only 
							//using the gravity data product between two processes on the same computer).  
							"tcp", 
							//Give an instance of the multiplication service class to be called when a request is made for multiplication.  
							msp);

		//Wait for us to exit (Ctrl-C or being killed).  
		gn.waitForExit();
		
		//Currently this will never be hit because we will have been killed (unfortunately).  
		//This tells the service directory that the multiplication service is no longer available.  
		gn.unregisterService("Multiplication");
	}


}
