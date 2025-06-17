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
import com.aphysci.gravity.GravityServiceProvider;
import com.aphysci.gravity.swig.GravityNode;
import com.aphysci.gravity.swig.SpdLog;
import com.aphysci.gravity.swig.SpdLog.LogLevel;
import com.aphysci.gravity.swig.GravityTransportType;

class MultiplicationServiceProvider implements GravityServiceProvider
{
	public  MultiplicationServiceProvider()
	{
	}

	@Override
	public GravityDataProduct request(String serviceID, GravityDataProduct dataProduct)
	{
		//Just to be safe.  In theory this can never happen unless this class is registered with more than one serviceID types.
		if(!dataProduct.getDataProductID().equals("Multiplication")) {
			SpdLog.critical(String.format("Request is not for %s, not Multiplication!", dataProduct.getDataProductID()));
			return new GravityDataProduct("BadRequest");
		}

		//Get the parameters for this request.
		Multiplication.MultiplicationOperandsPB.Builder params = Multiplication.MultiplicationOperandsPB.newBuilder();
		dataProduct.populateMessage(params);

		SpdLog.warn(String.format("%d x %d", params.getMultiplicandA(), params.getMultiplicandB()));

		//Do the calculation
		int result = params.getMultiplicandA() * params.getMultiplicandB();

		//Return the results to the requestor
		Multiplication.MultiplicationResultPB.Builder resultPB = Multiplication.MultiplicationResultPB.newBuilder();
		resultPB.setResult(result);

		GravityDataProduct resultDP = new GravityDataProduct("MultiplicationResult");
		resultDP.setData(resultPB);

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

		MultiplicationServiceProvider msp = new MultiplicationServiceProvider();
		gn.registerService(
							//This identifies the Service to the service directory so that others can
							// make a request to it.
							"Multiplication",
							//Assign a transport type to the socket (almost always tcp, unless you are only
							//using the gravity data product between two processes on the same computer).
							GravityTransportType.TCP,
							//Give an instance of the multiplication service class to be called when a request is made for multiplication.
							msp);

		//Wait for us to exit (Ctrl-C or being killed).
		gn.waitForExit();

		//Currently this will never be hit because we will have been killed (unfortunately).
		//This tells the service directory that the multiplication service is no longer available.
		gn.unregisterService("Multiplication");
	}


}
