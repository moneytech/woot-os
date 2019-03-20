#include <string.hpp>
#include <typeinfo>

std::type_info::~type_info()
{
}

const char *std::type_info::name() const
{
    return __name[0] == '*' ? __name + 1 : __name;
}

bool std::type_info::before(const std::type_info &arg) const
{
    return (name()[0] == '*') ? name() < arg.name() : String::Compare(name(), arg.name()) < 0;
}

bool std::type_info::operator ==(const std::type_info &arg) const
{
    return (&arg == this) || (__name[0] != '*' && !String::Compare(name(), arg.name()));
}

bool std::type_info::operator !=(const std::type_info &arg) const
{
    return !(operator ==(arg));
}

bool std::type_info::__is_pointer_p() const
{
    return false;
}

bool std::type_info::__is_function_p() const
{
    return false;
}

bool std::type_info::__do_catch(const type_info *thr_type, void **thr_obj, unsigned outer) const
{
    return *this == *thr_type;
}

bool std::type_info::__do_upcast(const __cxxabiv1::__class_type_info *target, void **obj_ptr) const
{
    return false;
}

std::type_info::type_info(const char *n) :
    __name(n)
{
}
