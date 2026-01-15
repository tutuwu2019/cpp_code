#pragma once

#include <string>

namespace db
{

    enum class ErrorCode
    {
        Ok = 0,
        Timeout,
        NotImplemented,
        NotConnected,
        InvalidArgument,
        DriverError,
    };

    struct Status
    {
        ErrorCode code{ErrorCode::Ok};
        std::string message;

        static Status success() { return {}; }
        static Status timeout(std::string msg = "timeout") { return {ErrorCode::Timeout, std::move(msg)}; }
        static Status invalid_argument(std::string msg) { return {ErrorCode::InvalidArgument, std::move(msg)}; }
        static Status not_connected(std::string msg = "not connected") { return {ErrorCode::NotConnected, std::move(msg)}; }
        static Status not_implemented(std::string msg = "not implemented") { return {ErrorCode::NotImplemented, std::move(msg)}; }
        static Status driver_error(std::string msg) { return {ErrorCode::DriverError, std::move(msg)}; }

        bool ok() const { return code == ErrorCode::Ok; }
    };

} // namespace db
