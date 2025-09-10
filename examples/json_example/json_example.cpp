#include <aetherium/format/json.hpp>

#include <iostream>

int main() {
  using namespace Aetherium::Format;

  std::string json_str =
      "{\"name\": \"Alice\", \"age\": 30, \"isStudent\": false, "
      "\"courses\": [\"Math\", \"Physics\", \"Chemistry\"], "
      "\"address\": {\"street\": \"123 Main St\", \"city\": \"Anytown\"}, "
      "\"grades\": null, \"gpa\": 3.85, \"empty_array\": [], "
      "\"empty_object\": {}, \"escaped_string\": "
      "\"Hello, \\\"World\\\"!\\nNew line.\"}";

  JsonParser parser;
  try {
    JsonValue json_doc = parser.parse(json_str);

    std::cout << "Name: " << json_doc["name"].get_string() << std::endl;

    std::cout << "Age: " << static_cast<int>(json_doc["age"].get_number())
              << std::endl;

    std::cout << "Is Student: "
              << (json_doc["isStudent"].get_bool() ? "true" : "false")
              << std::endl;

    std::cout << "GPA: " << json_doc["gpa"].get_number() << std::endl;

    std::cout << "Grades is null: "
              << (json_doc["grades"].is_null() ? "true" : "false") << std::endl;

    std::cout << "Courses: ";
    const JsonArray &courses = json_doc["courses"].get_array();

    for (const auto &course : courses)
      std::cout << course.get_string() << " ";
    std::cout << std::endl;

    std::cout << "Address Street: "
              << json_doc["address"]["street"].get_string() << std::endl;

    std::cout << "Address City: " << json_doc["address"]["city"].get_string()
              << std::endl;

    std::cout << "Empty Array is array: " << json_doc["empty_array"].is_array()
              << std::endl;

    std::cout << "Empty Object is object: "
              << json_doc["empty_object"].is_object() << std::endl;

    std::cout << "Escaped string: " << json_doc["escaped_string"].get_string()
              << std::endl;

    json_doc["age"] = 31;
    json_doc["new_field"] = "This is a new value";
    json_doc["courses"][0] = "Calculus";
    json_doc["courses"].get_array().push_back("Data Structures");

    std::cout << "\n--- Pretty Printed JSON (Modified) ---" << std::endl;
    std::cout << json_doc.serialize(true) << std::endl;

    std::cout << "\n--- Compact JSON (Modified) ---" << std::endl;
    std::cout << json_doc.serialize(false) << std::endl;

    std::string complex_json = "{\"data\": {\"items\": [{\"id\": 1, "
                               "\"name\": \"Item A\"}, {\"id\": 2, "
                               "\"name\": \"Item B\"}], \"count\": 2}}";

    JsonValue complex_doc = parser.parse(complex_json);
    std::cout << "\n--- Complex JSON ---" << std::endl;
    std::cout << complex_doc.serialize(true) << std::endl;

    int first_item_id =
        static_cast<int>(complex_doc["data"]["items"][0]["id"].get_number());

    std::string second_item_name =
        complex_doc["data"]["items"][1]["name"].get_string();

    std::cout << "First item ID: " << first_item_id << std::endl;
    std::cout << "Second item name: " << second_item_name << std::endl;

    JsonValue dynamic_json;
    dynamic_json["some_array"][0] = "first element";
    dynamic_json["some_array"][1] = 123;
    dynamic_json["some_object"]["key"] = true;
    dynamic_json["some_object"]["another_key"] = JsonValue();

    std::cout << "\n--- Dynamic JSON ---" << std::endl;
    std::cout << dynamic_json.serialize(true) << std::endl;
  } catch (const JsonParseException &e) {
    std::cerr << "JSON Parsing Error: " << e.what() << std::endl;
  } catch (const std::bad_variant_access &e) {
    std::cerr << "JSON Type Access Error (std::bad_variant_access): "
              << e.what() << std::endl;
  } catch (const std::out_of_range &e) {
    std::cerr << "JSON Out of Range Error: " << e.what() << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
  }

  std::vector<std::string> invalid_jsons = {"{\"key\": \"value\" extra}",
                                            "{\"key\": \"value\",}",
                                            "{\"key\": 1.2.3}",
                                            "{\"key\": [1, 2, }]}",
                                            "{\"key\": \"unterminated string}",
                                            "nul",
                                            "{bad_key: 1}",
                                            "{\"key\": \"\x01\"}"};

  std::cout << "\n--- Error Handling Tests ---" << std::endl;
  for (size_t i = 0; i < invalid_jsons.size(); ++i)
    try {
      std::cout << "Parsing invalid JSON " << i + 1 << ": " << invalid_jsons[i]
                << std::endl;

      JsonValue doc = parser.parse(invalid_jsons[i]);
      std::cout << "  (Unexpectedly succeeded: " << doc.serialize() << ")"
                << std::endl;
    } catch (const JsonParseException &e) {
      std::cerr << "  Caught expected error: " << e.what() << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "  Caught unexpected error type: " << e.what() << std::endl;
    }

  return 0;
}
