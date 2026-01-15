#pragma once

#include "db/db_error.h"

#include <utility>

namespace db
{

    template <class T>
    struct Result
    {
        Status status;
        T value{};

        static Result<T> ok(T v) { return {Status::success(), std::move(v)}; }
        static Result<T> error(Status s) { return {std::move(s), T{}}; }

        bool ok() const { return status.ok(); }
    };

} // namespace db
