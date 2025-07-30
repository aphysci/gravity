#include "RustFutureResponse.h"

namespace gravity {

    std::shared_ptr<FutureResponse> rustNewFutureResponse(const char* arrayPtr, int size)
    {
        return std::shared_ptr<FutureResponse>(new FutureResponse(arrayPtr, size));
    }

    void rustSetResponse(const std::unique_ptr<FutureResponse>& fr, const std::unique_ptr<GravityDataProduct>& response)
    {
        fr->setResponse(*response);
    }


} // namespace gravity