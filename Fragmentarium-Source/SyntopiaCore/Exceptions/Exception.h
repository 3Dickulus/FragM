#pragma once
#include <QException>

namespace SyntopiaCore
{
namespace Exceptions
{

/// A base exception class.
///
/// When using Exceptions:
///  (1) Throw temporaries   (throw Exception("Error occoured");)
///  (2) Catch by reference  ( try {} catch (Exception& e) {} )
///
/// (Perhaps this ought to inherit from std::exception?)
class Exception : public QException
{

public:
    /// Constructor.
    Exception ( QString message ) : message ( message ) {}

    void raise() const
    {
        throw *this;
    }
    Exception *clone() const
    {
        return new Exception ( *this );
    }
    /// Returns the error message.
    QString getMessage() const
    {
        return message;
    }

private:
    QString message;
};

} // namespace Exceptions
} // namespace SyntopiaCore
