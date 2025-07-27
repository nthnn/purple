#include <aetherium/format/dotenv.hpp>

#include <fstream>
#include <iostream>

int main() {
    using namespace Aetherium::Format;

    std::ofstream test_env_file(".env");
    if(test_env_file.is_open()) {
        test_env_file << "# This is a comment\n";
        test_env_file << "APP_NAME=MyAwesomeApp\n";
        test_env_file << "DB_HOST=localhost\n";
        test_env_file << "DB_PORT=5432\n";
        test_env_file << "API_KEY=\"some_secret_key_with_spaces\"\n";
        test_env_file << "DEBUG=true\n";
        test_env_file << "EMPTY_VAR=\n";
        test_env_file << "MULTI_LINE_STRING=\"Hello\\nWorld!\"\n";
        test_env_file << "SINGLE_QUOTED='Another value with spaces'\n";
        test_env_file << "  SPACED_KEY  =  SPACED_VALUE  \n";
        test_env_file << "\n";
        test_env_file << "MALFORMED_LINE\n";
        test_env_file.close();

        std::cout << "Created a dummy .env file for testing."
            << std::endl
            << std::endl;
    }
    else {
        std::cerr << "Error: Could not create dummy .env file."
            << std::endl;
        return 1;
    }

    DotEnv env;
    if(env.load(".env")) {
        std::cout << "Successfully loaded .env file." << std::endl;

        try {
            std::cout << "APP_NAME: "
                << env.get("APP_NAME")
                << std::endl;

            std::cout << "DB_HOST: "
                << env.get("DB_HOST")
                << std::endl;

            std::cout << "DB_PORT: "
                << env.get("DB_PORT")
                << std::endl;

            std::cout << "API_KEY: "
                << env.get("API_KEY")
                << std::endl;

            std::cout << "DEBUG: "
                << env.get("DEBUG")
                << std::endl;

            std::cout << "EMPTY_VAR: '"
                << env.get("EMPTY_VAR")
                << "'"
                << std::endl;

            std::cout << "MULTI_LINE_STRING: '"
                << env.get("MULTI_LINE_STRING")
                << "'"
                << std::endl;

            std::cout << "SINGLE_QUOTED: '"
                << env.get("SINGLE_QUOTED")
                << "'"
                << std::endl;

            std::cout << "SPACED_KEY: '"
                << env.get("SPACED_KEY")
                << "'"
                << std::endl;

            std::cout << "NON_EXISTENT_VAR (with default): "
                << env.get("NON_EXISTENT_VAR", "default_value")
                << std::endl;
        }
        catch(const std::out_of_range& e) {
            std::cerr << "Error accessing variable: "
                << e.what()
                << std::endl;
        }
    }
    else {
        std::cerr << "Failed to load .env file."
            << std::endl;
        return 1;
    }

    std::remove(".env");
    std::cout << "\nCleaned up dummy .env file."
        << std::endl;

    return 0;
}
