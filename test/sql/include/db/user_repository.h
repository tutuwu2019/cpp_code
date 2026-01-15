#pragma once

#include "db/db_helper.h"
#include "db/db_result.h"
#include "db/user.h"

#include <optional>
#include <vector>

namespace db
{

    class UserRepository
    {
    public:
        explicit UserRepository(DBHelper &helper);

        Result<int64_t> create(const User &u);
        Result<User> getById(int64_t id);
        Result<std::vector<User>> query(const UserQuery &q);

        Result<int64_t> updateById(const User &u); // affectedRows
        Result<int64_t> deleteById(int64_t id);    // affectedRows

    private:
        DBHelper &helper_;
    };

} // namespace db
