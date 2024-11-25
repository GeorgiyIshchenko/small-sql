
#include <gtest/gtest.h>

#include <Database.hpp>

#include <filesystem>

const std::filesystem::path dbPath{ "../db/example.db" };

TEST(Operation, Complex)
{

#ifdef DEBUG
    std::cout << "db path: " << std::filesystem::absolute(dbPath) << std::endl;
#endif

    db::Database::getInstance().execute(
        "create table users ({key, autoincrement} id : int32, {unique} login: "
        "string[32], password_hash: bytes[8], is_admin: bool = false)");

    db::Database::getInstance().execute(
        "insert (login = \"gosha\", password_hash = 0xdeadbeefdeadbeef, "
        "is_admin = true) to users");

    db::Database::getInstance().execute(
        "insert (login = \"gosha_vtoroy\", password_hash = 0xbeefdead, "
        "is_admin = false) to users");

    db::Database::getInstance().storeTableInFile("users", dbPath);
}
