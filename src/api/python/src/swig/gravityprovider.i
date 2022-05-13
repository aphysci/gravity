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

// this turns on director features for GravityServiceProvider
%feature("director") gravity::GravityServiceProvider;

// This does most of the work to convert the C++ Vector of GDP's to Python objects.  This C++
// code will create a Python list of Python string objects containing a binary serialization of 
// the GDP protobuf.  Instead of trying to instantiate a Python GravityDataProduct in this C++ 
// code, it seems cleaner to leave that to the Python code (defined below).  
%typemap(directorin) const gravity::GravityDataProduct& dataProduct {
    char* buffer = new char[dataProduct.getSize()];
    dataProduct.serializeToArray(buffer);
    $input = PyBytes_FromStringAndSize(buffer, dataProduct.getSize());
    delete[] buffer;
}

%typemap(directorout) std::shared_ptr<gravity::GravityDataProduct> {
    char* buffer = PyBytes_AsString($input);
    Py_ssize_t bufferLen = PyBytes_Size($input);
    std::shared_ptr<gravity::GravityDataProduct> resultDP(new gravity::GravityDataProduct(buffer, bufferLen));
    $result = resultDP;
}

namespace gravity {

    %pythonprepend GravityServiceProvider::GravityServiceProvider() %{
    # check for > 2 parents
    gravityParents = set(["GravitySubscriber", "GravityServiceProvider",
                          "GravityRequestor",  "GravityHeartbeatListener"])
    numExtended = len(gravityParents.intersection([c.__name__ for c in self.__class__.__bases__]))
    if numExtended > 2:
        raise ValueError("\n\nCurrently only extending 2 Gravity classes is supported, but extending {}: \n\t\t{}.\nSee https://github.com/aphysci/gravity/issues/190.\n\n".format(numExtended, self.__class__.__bases__))
    %}
    
    // rename the method defined in the C++ GravityServiceProvider so that the callback from C++ will invoke the requestBinary defined below
    %rename(requestBinary) GravityServiceProvider::request;
    
    // This says request, but it will show up as requestBinary in the generated code due to the rename above.  This
    // defines the method that will be invoked by the C++ callback.  The C++ code (defined above) converts everything to Python objects, but
    // I think this is the cleanest way to generate GravityDataProduct objects from the binary strings created in the C++ code.
    %feature("shadow") GravityServiceProvider::request(const std::string serviceID, const gravity::GravityDataProduct& dataProduct) %{
    
        # DO NOT override or alter this method. It is here to convert between a serialized, binary Python string 
        # representing a GravityDataProduct to a GravityDataProduct Python object.
        def requestBinary(self, *args):
            serviceID = args[0]
            try:
                request = GravityDataProduct(data=args[1])
                try:
                    response = self.request(serviceID, request)
                except Exception as e:
                    logging.exception("Exception caught calling client request")
            except Exception as e:
                logging.exception("Error creating GDP from byte array")
            return response.serializeToString()
    %}

	class GravityServiceProvider
	{
	public:
	    /**
	     * Default destructor
	     */
    virtual ~GravityServiceProvider();

    /**
     * Called when a request is made through the Gravity infrastructure
     * \param serviceID service ID of the requesting service
     * \param dataProduct GravityDataProduct with request data
     * \returns the response
     */
    virtual std::shared_ptr<gravity::GravityDataProduct> request(const std::string serviceID, const gravity::GravityDataProduct& dataProduct) = 0;
	    
    %pythoncode %{
	    # The above request will be renamed as requestBinary, and will be invoked from C++ when a service request is made.
	    # Python GravityServiceProvider child classes should only override this abstract method.
	    @abc.abstractmethod
	    def request(self, serviceID, dataProduct): pass
    %}
	    
	};

};
