#include <iostream>

// Use absolute include paths
#include "C:/vcpkg/installed/x64-windows/include/mongocxx/v_noabi/mongocxx/client.hpp"
#include "C:/vcpkg/installed/x64-windows/include/mongocxx/v_noabi/mongocxx/instance.hpp"
#include "C:/vcpkg/installed/x64-windows/include/mongocxx/v_noabi/mongocxx/uri.hpp"
#include "C:/vcpkg/installed/x64-windows/include/bsoncxx/v_noabi/bsoncxx/builder/stream/document.hpp"
#include "C:/vcpkg/installed/x64-windows/include/bsoncxx/v_noabi/bsoncxx/json.hpp"

int main() {
    std::cout << "Testing MongoDB..." << std::endl;

    try {
        mongocxx::instance inst{};
        mongocxx::client client{ mongocxx::uri{"mongodb://localhost:27017"} };

        auto db = client["game_db"];
        auto collection = db["players"];

        auto doc = bsoncxx::builder::stream::document{}
            << "player_id" << "TestPlayer1"
            << "position" << bsoncxx::builder::stream::open_document
            << "x" << 10.5
            << "y" << 20.3
            << "z" << 0.0
            << bsoncxx::builder::stream::close_document
            << "health" << 100
            << bsoncxx::builder::stream::finalize;

        collection.insert_one(doc.view());
        std::cout << "SUCCESS! Data saved to MongoDB!" << std::endl;

    }
    catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << std::endl;
    }

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    return 0;
}
