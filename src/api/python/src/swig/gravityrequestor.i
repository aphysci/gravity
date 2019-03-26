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

// this turns on director features for GravityRequestor
%feature("director") gravity::GravityRequestor;

// This does most of the work to convert the C++ Vector of GDP's to Python objects.  This C++
// code will create a Python list of Python string objects containing a binary serialization of 
// the GDP protobuf.  Instead of trying to instantiate a Python GravityDataProduct in this C++ 
// code, it seems cleaner to leave that to the Python code (defined below).  
%typemap(directorin) const gravity::GravityDataProduct& response {
    char* buffer = new char[response.getSize()];
    response.serializeToArray(buffer);
    $input = PyString_FromStringAndSize(buffer, response.getSize());
    delete[] buffer;
}

namespace gravity {

    // rename the method defined in the C++ GravityRequestor so that the callback from C++ will invoke the requestFilledBinary defined below
    %rename(requestFilledBinary) GravityRequestor::requestFilled;
    
    // This says requestFilled, but it will show up as requestFilledBinary in the generated code due to the rename above.  This
    // defines the method that will be invoked by the C++ callback.  The C++ code (defined above) converts everything to Python objects, but
    // I think this is the cleanest way to generate GravityDataProduct objects from the binary strings created in the C++ code.
    %feature("shadow") GravityRequestor::requestFilled(std::string serviceID, std::string requestID, const gravity::GravityDataProduct& response) %{
    
        # DO NOT override or alter this method. It is here to convert between a serialized, binary Python string 
        # representing a GravityDataProduct to a GravityDataProduct Python object.
        def requestFilledBinary(self, *args):
            serviceID = args[0]
            requestID = args[1]
            try:
                response = GravityDataProduct(data=args[2])
                try:
                    self.requestFilled(serviceID, requestID, response)
                except Exception as e:
                    logging.exception("Exception caught calling client requestFilled")
            except Exception as e:
                logging.exception("Error creating GDP from byte array")
    %}

// This injects code at the end of both director methods generated for the GravityRequestor.  Once either of these callbacks are invoked,
// Gravity is done with this object, so we can safely decrement the refcount.
%typemap(directorargout) (std::string serviceID, std::string requestID) {

// SWIG_PYTHON_DIRECTOR_VTABLE should never be defined in our case, but include the check because swig_get_self is invalid if that's set.
%#if !defined(SWIG_PYTHON_DIRECTOR_VTABLE)

    // Decrement the refcount on the requestor object now that callback has been invoked
    Py_XDECREF(swig_get_self());
%#endif
}

	class GravityRequestor
	{
	public:
	    /**
	     * Default destructor
	     */
	    virtual ~GravityRequestor();
	
	    /**
	     * Called when a response to a request is received through the Gravity infrastructure
	     * \param serviceID ID of the service request is being filled through
	     * \param requestID ID of the request that was made
	     * \param response GravityDataProduct containing the data of the response
	     */
	    virtual void requestFilled(std::string serviceID, std::string requestID, const gravity::GravityDataProduct& response) = 0;
	    
    %pythoncode %{
	    # The above requestFilled will be renamed as requestFilledBinary, and will be invoked from C++ when a response is available.
	    # Python GravityRequestor child classes should only override this abstract method.
	    @abc.abstractmethod
	    def requestFilled(self, serviceID, requestID, response): pass
    %}

	    virtual void requestTimeout(std::string serviceID, std::string requestID);
	    
	};

};