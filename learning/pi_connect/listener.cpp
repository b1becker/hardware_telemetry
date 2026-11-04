// start a crow http server that listen on port 18080. 
// clinet sends GET request to port --> crow routes to handler -- > 
// hander return response

#include "crow.h"
#include <string>
#include <iostream>

crow::json::wvalue handleRequest();

// handoff: us this struct to package the metric path
struct RouteRequest {
    std::string path;
};

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/redfish/v1/Systems/1")([](){
        return handleRequest();
    });

    app.port(18080).multithreaded().run();
}

//abstracts away the response handling in json
// this is going to need arg: path and map. path is to match which metric
// and map is to find the store where all the redfish to arm64/x86 metrics are
// NOTE: I think we need a normalization layer too where we have to do
//      format conversions.
crow::json::wvalue handleRequest()
{
    crow::json::wvalue response;
    response["@odata.id"] = "/redfish/v1/Systems/1";
    response["Id"] = "1";
    response["Name"] = "Edge System";

    return response;
}