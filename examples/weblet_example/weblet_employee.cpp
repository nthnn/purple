#include <aetherium/net/weblet.hpp>
#include <iostream>

using namespace Aetherium::Net;

WebletDynamicHandler employee_fetch(
    Request request,
    std::map<std::string, std::string> parameters
) {
    Response response;
    response.set_header("Content-Type", "application/json");

    std::string employee_id;
    if(parameters.count("id"))
        employee_id = parameters["id"];

    if(employee_id.empty()) {
        response.status_code = 200;
        response.contents =
            "{\"message\": \"No employee ID provided.\", "
            "\"example_url\": \"/api/dynamic-employee/123\"}";

            std::cout << "  Dynamic Employee handler: No ID provided"
                << std::endl;
    }
    else {
        response.contents = std::string("{\"employee_id\": \"") +
            employee_id +
            "\", \"name\": \"Dynamic John Doe\", "
            "\"position\": \"Dynamic Software Engineer\"}";
        response.status_code = 200;

        std::cout << "  Dynamic Employee handler executed for ID: "
            << employee_id
            << std::endl;
    }
    
    return response;
}
