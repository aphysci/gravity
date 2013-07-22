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

import java.util.List;
import com.aphysci.gravity.GravityDataProduct;
import com.aphysci.gravity.GravitySubscriber;
import com.aphysci.gravity.swig.GravityNode;
import com.aphysci.gravity.swig.Log;
import com.aphysci.gravity.swig.Log.LogLevel;


class SimpleGravityCounterSubscriber implements GravitySubscriber
{
	@Override
	public void subscriptionFilled(List<GravityDataProduct> dataProducts)
	{
		for (GravityDataProduct dataProduct : dataProducts) {
			//Get the protobuf object from the message
			BasicCounterDataProduct.BasicCounterDataProductPB.Builder counterDataPB = BasicCounterDataProduct.BasicCounterDataProductPB.newBuilder();
			if(!dataProduct.populateMessage(counterDataPB))
				Log.warning("Error Parsing Message");
			
			//Process the message
			Log.warning(String.format("Current Count: %d", counterDataPB.getCount()));
		}
	}
}


public class JavaProtobufDataProductSubscriber {
	
	/**
	 * @param args
	 */
	public static void main(String[] args) {
		GravityNode gn = new GravityNode();
		//Initialize gravity, giving this node a componentID.  
		gn.init("JavaProtobufDataProductSubscriber");

		//Declare an object of type SimpleGravityCounterSubscriber (this also initilizes the total count to 0).  
		SimpleGravityCounterSubscriber counterSubscriber = new SimpleGravityCounterSubscriber();
		//Subscribe a SimpleGravityCounterSubscriber to the counter data product.  
		gn.subscribe("BasicCounterDataProduct", counterSubscriber); 

		//Wait for us to exit (Ctrl-C or being killed).  
		gn.waitForExit();

		//Currently this will never be hit because we will have been killed (unfortunately).  
		//But this shouldn't make a difference because the OS should close the socket and free all resources.  
		gn.unsubscribe("BasicCounterDataProduct", counterSubscriber);	


	}


}
