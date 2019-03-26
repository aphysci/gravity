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

// this turns on director features for GravitySubscriber
%feature("director") gravity::GravitySubscriber;

// This does most of the work to convert the C++ Vector of GDP's to Python objects.  This C++
// code will create a Python list of Python string objects containing a binary serialization of 
// the GDP protobuf.  Instead of trying to instantiate a Python GravityDataProduct in this C++ 
// code, it seems cleaner to leave that to the Python code (defined below).  
%typemap(directorin) const std::vector< std::tr1::shared_ptr<gravity::GravityDataProduct> >& dataProducts {
    $input = PyList_New(dataProducts.size());
    size_t maxSize = 0;
    for (unsigned int i = 0; i < dataProducts.size(); i++)
    {
        size_t s = dataProducts.at(i)->getSize();
        if (s > maxSize) maxSize = s;
    }
    char* buffer = new char[maxSize];
    for (unsigned int i = 0; i < dataProducts.size(); i++)
    {
        std::tr1::shared_ptr<gravity::GravityDataProduct> dataProduct = dataProducts.at(i);
        dataProduct->serializeToArray(buffer);
        PyObject* pyStr = PyBytes_FromStringAndSize(buffer, dataProduct->getSize());
        PyList_SetItem($input, i, pyStr);
    }
    delete[] buffer;
}

namespace gravity {

    // rename the method defined in the C++ GravitySubscriber so that the callback from C++ will invoke the subscriptionFilledBinary defined below
    %rename(subscriptionFilledBinary) GravitySubscriber::subscriptionFilled;
    
    // This says subscriptionFilled, but it will show up as subscriptionFilledBinary in the generated code due to the rename above.  This
    // defines the method that will be invoked by the C++ callback.  The C++ code (defined above) converts everything to Python objects, but
    // I think this is the cleanest way to generate GravityDataProduct objects from the binary strings created in the C++ code.
    %feature("shadow") GravitySubscriber::subscriptionFilled(const std::vector< std::tr1::shared_ptr<gravity::GravityDataProduct> >& dataProducts) %{
    
        # DO NOT override or alter this method. It is here to convert between a list of serialized, binary Python strings 
        # representing GravityDataProduct's to a list of GravityDataProduct Python objects.
        def subscriptionFilledBinary(self, *args):
            gdpList = []
            for byteStr in args[0]:
                try:
                    gdpList.append(GravityDataProduct(data=byteStr))
                except Exception as e:
                    logging.exception("Error creating GDP from byte array")
            try:
                self.subscriptionFilled(gdpList)
            except Exception as e:
                logging.exception("Exception caught calling client subscriptionFilled")
    %}

	class GravitySubscriber {
	public:
	    virtual ~GravitySubscriber();
	    virtual void subscriptionFilled(const std::vector< std::tr1::shared_ptr<gravity::GravityDataProduct> >& dataProducts) = 0;
	    
	%pythoncode %{
	    # The above subscriptionFilled will be renamed as subscriptionFilledBinary, and will be invoked from C++ when data is available.
	    # Python GravitySubscriber child classes should only override this abstract method, which will be called with a list of
	    # GravityDataProduct objects.
	    @abc.abstractmethod
	    def subscriptionFilled(self, dataProducts): pass
    %}
	};
};
