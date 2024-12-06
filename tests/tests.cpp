
#include <gtest/gtest.h>

#include <Database.hpp>

#include <filesystem>

const std::filesystem::path exampleDbPath{ "../db/example.db" };

TEST(Operation, Complex)
{

#ifdef DEBUG
    std::cout << "db path: " << std::filesystem::absolute(exampleDbPath) << std::endl;
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

    db::Database::getInstance().execute(
        "insert (login = \"gosha_treriy\", password_hash = 0xbeefdead, "
        "is_admin = false) to users");

    db::Database::getInstance().execute("SELECT * FROM users");

    db::Database::getInstance().execute("SELECT id, login FROM users");

    db::Database::getInstance().execute(
        "SELECT id, login FROM users where id = 1");

    db::Database::getInstance().execute("SELECT * FROM users where id = 1 + 1");

    db::Database::getInstance().execute("SELECT * FROM users where login = gosha");

    db::Database::getInstance().execute(
        "SELECT id, login FROM users where id <= 1");

    db::Database::getInstance().execute(
        "update users set is_admin = true where id = 1");

    db::Database::getInstance().execute(
        "insert (login = \"to_delete\", password_hash = 0xbeefdead, "
        "is_admin = false) to users");

    db::Database::getInstance().execute("delete users where login = to_delete");

    db::Database::getInstance().storeTableInFile("users", exampleDbPath);
}

TEST(Operation, LoadStore){

    EXPECT_NO_THROW(db::Database::getInstance().loadTableFromFile("users", exampleDbPath));

    EXPECT_NO_THROW(db::Database::getInstance().storeTableInFile("users", std::filesystem::path{"../db/example_copy.db"}));

}
