#ifndef RUSTFUTURERESPONSE_H_
#define RUSTFUTURERESPONSE_H_


#include <FutureResponse.h>



namespace gravity
{
    /* Future response things*/
    std::shared_ptr<FutureResponse> rustNewFutureResponse(const char* arrayPtr, int size);

    void rustSetResponse(const std::unique_ptr<FutureResponse>& fr, const std::unique_ptr<GravityDataProduct>& response);

    
    
} // namespace gravity


#endif