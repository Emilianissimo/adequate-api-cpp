//
// Created by user on 11.02.2026.
//

#ifndef BEAST_API_FILESYSTEMERRORS_H
#define BEAST_API_FILESYSTEMERRORS_H

#pragma once
#include <stdexcept>

class FileError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class FileValidationError final : public FileError
{
public:
    using FileError::FileError;
};


class FileSecurityError final : public FileError {
public:
    using FileError::FileError;
};

class FileIOError final : public FileError {
public:
    using FileError::FileError;
};

#endif //BEAST_API_FILESYSTEMERRORS_H
