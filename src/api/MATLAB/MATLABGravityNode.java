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

package com.aphysci.gravity.matlab;
import com.aphysci.gravity.swig.GravityNode;
import com.aphysci.gravity.swig.GravityReturnCode;


public class MATLABGravityNode extends com.aphysci.gravity.swig.GravityNode
{
	public MATLABGravityNode()
	{
		super();
	}

	public MATLABGravityNode(String nodename)
	{
		super(nodename);
	}

	public Object[] subscribersExist(String dataProductID)
	{
		boolean[] bools = new boolean[1];
		GravityReturnCode code = super.subscribersExist(dataProductID, bools);
		Object[] ret = new Object[2];
		ret[0] = code;
		ret[1] = bools[0];
		return ret;
	}
}
