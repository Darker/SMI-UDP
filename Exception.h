#ifndef EXCEPTION_H
#define EXCEPTION_H
#include <QString>
class Exception: public std::exception
{
public:
    /** Constructor (C strings).
     *  @param message C-style string error message.
     *                 The string contents are copied upon construction.
     *                 Hence, responsibility for deleting the char* lies
     *                 with the caller.
     */
    explicit Exception(const char* message):
      msg_(message)
      {
      }

    /** Constructor (C++ STL strings).
     *  @param message The error message.
     */
    explicit Exception(const std::string& message):
      msg_(QString::fromStdString(message))
      {}
    explicit Exception(const QString& message):
      msg_(message)
      {}
    Exception():
      msg_("Unknown error!")
      {}
    /** Destructor.
     * Virtual to allow for subclassing.
     */
    virtual ~Exception() throw (){}

    /** Returns a pointer to the (constant) error description.
     *  @return A pointer to a const char*. The underlying memory
     *          is in posession of the Exception object. Callers must
     *          not attempt to free the memory.
     */
    virtual const char* what() const throw (){
       return msg_.toStdString().c_str();
    }

protected:
    /** Error message.
     */
    QString msg_;
};
#endif // EXCEPTION_H
