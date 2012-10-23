package com.aphysci.gravity.matlab;

import com.aphysci.gravity.GravitySubscriber;
import com.aphysci.gravity.GravityDataProduct;
import java.util.Vector;
import java.util.List;

public class MATLABGravitySubscriber implements GravitySubscriber
{
	private Vector<GravityDataProduct> data = new Vector<GravityDataProduct>();	

	public void subscriptionFilled(final GravityDataProduct dataProduct)
	{
		data.addElement(dataProduct);		
	}

	public GravityDataProduct getDataProduct(int timeoutMS)
	{
		GravityDataProduct gdp = null;
		do
		{			
			if (!data.isEmpty())
			{
				gdp = data.elementAt(0);
				data.removeElementAt(0);			
			}
			else if (timeoutMS < 0)
			{
				try {Thread.sleep(10);} catch (InterruptedException ie) {}
			}
			else
			{
				int sleepTime = Math.min(timeoutMS, 10);
				try {Thread.sleep(sleepTime);} catch (InterruptedException ie) {}
				timeoutMS -= sleepTime;
			}
		} while (timeoutMS != 0 && gdp == null);

		return gdp;
	}

	public Vector<GravityDataProduct> getAllDataProducts()
	{
		@SuppressWarnings("unchecked") 
		Vector<GravityDataProduct> ret = (Vector<GravityDataProduct>)data.clone();
		data.clear();
		return ret;
	}

	static public Double[] convertDoubleListToDoubleArray(List<Double> list)
	{
		Double[] a = new Double[list.size()];
		list.toArray(a);
		return a;
	}
}
