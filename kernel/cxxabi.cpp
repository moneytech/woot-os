#include <cxxabi.h>

#define offsetof __builtin_offsetof

struct vtable_prefix
{
    __PTRDIFF_TYPE__ whole_object;
    const __cxxabiv1::__class_type_info *whole_type;
    const void *origin;
};

static const __cxxabiv1::__class_type_info *const nonvirtual_base_type = static_cast <const __cxxabiv1::__class_type_info *> (0) + 1;

static bool contained_public_p(__cxxabiv1::__class_type_info::__sub_kind access_path)
{
    return ((access_path & __cxxabiv1::__class_type_info::__contained_public) == __cxxabiv1::__class_type_info::__contained_public);
}

template <typename T>
static const T *adjust_pointer(const void *base, __PTRDIFF_TYPE__ offset)
{
    return reinterpret_cast<const T *>(reinterpret_cast<const char *>(base) + offset);
}

static void const *convert_to_base(void const *addr, bool is_virtual, __PTRDIFF_TYPE__ offset)
{
    if(is_virtual)
    {
        const void *vtable = *static_cast<const void *const *>(addr);
        offset = *adjust_pointer<__PTRDIFF_TYPE__>(vtable, offset);
    }
    return adjust_pointer<void>(addr, offset);
}

static bool contained_p(__cxxabiv1::__class_type_info::__sub_kind access_path)
{
    return access_path >= __cxxabiv1::__class_type_info::__contained_mask;
}

static bool public_p(__cxxabiv1::__class_type_info::__sub_kind access_path)
{
    return access_path & __cxxabiv1::__class_type_info::__contained_public_mask;
}

static bool virtual_p(__cxxabiv1::__class_type_info::__sub_kind access_path)
{
    return (access_path & __cxxabiv1::__class_type_info::__contained_virtual_mask);
}

static bool contained_nonvirtual_p(__cxxabiv1::__class_type_info::__sub_kind access_path)
{
    return ((access_path & (__cxxabiv1::__class_type_info::__contained_mask | __cxxabiv1::__class_type_info::__contained_virtual_mask)) == __cxxabiv1::__class_type_info::__contained_mask);
}

extern "C" void *__dynamic_cast(const void* src_ptr, const __cxxabiv1::__class_type_info* src_type, const __cxxabiv1::__class_type_info* dst_type, __PTRDIFF_TYPE__ src2dst)
{
    const void *vtable = *static_cast <const void *const *> (src_ptr);
    const vtable_prefix *prefix = adjust_pointer<vtable_prefix>(vtable, -offsetof(vtable_prefix, origin));
    const void *whole_ptr = adjust_pointer<void>(src_ptr, prefix->whole_object);
    const __cxxabiv1::__class_type_info *whole_type = prefix->whole_type;
    __cxxabiv1::__class_type_info::__dyncast_result result;

    const void *whole_vtable = *static_cast<const void *const *>(whole_ptr);
    const vtable_prefix *whole_prefix = adjust_pointer<vtable_prefix>(whole_vtable, -offsetof(vtable_prefix, origin));
    if(whole_prefix->whole_type != whole_type)
        return nullptr;

    whole_type->__do_dyncast(src2dst, __cxxabiv1::__class_type_info::__contained_public, dst_type, whole_ptr, src_type, src_ptr, result);

    if(!result.dst_ptr)
        return nullptr;
    if(contained_public_p(result.dst2src))
        return const_cast<void *>(result.dst_ptr);
    if(contained_public_p(__cxxabiv1::__class_type_info::__sub_kind(result.whole2src & result.whole2dst)))
        return const_cast<void *>(result.dst_ptr);
    if(contained_nonvirtual_p(result.whole2src))
        return nullptr;
    if(result.dst2src == __cxxabiv1::__class_type_info::__unknown)
        result.dst2src = dst_type->__find_public_src(src2dst, result.dst_ptr, src_type, src_ptr);
    if(contained_public_p(result.dst2src))
        return const_cast<void *>(result.dst_ptr);
    return nullptr;
}

__cxxabiv1::__fundamental_type_info::__fundamental_type_info(const char *n) :
    std::type_info(n)
{
}

__cxxabiv1::__fundamental_type_info::~__fundamental_type_info()
{
}

__cxxabiv1::__array_type_info::__array_type_info(const char *n) :
    std::type_info(n)
{
}

__cxxabiv1::__array_type_info::~__array_type_info()
{
}

__cxxabiv1::__function_type_info::__function_type_info(const char *n) :
    std::type_info(n)
{
}

__cxxabiv1::__function_type_info::~__function_type_info()
{
}

bool __cxxabiv1::__function_type_info::__is_function_p() const
{
    return true;
}

__cxxabiv1::__enum_type_info::__enum_type_info(const char *n) :
    std::type_info(n)
{
}

__cxxabiv1::__enum_type_info::~__enum_type_info()
{
}

__cxxabiv1::__pbase_type_info::__pbase_type_info(const char *n, int quals, const std::type_info *type) :
    std::type_info(n), __flags(quals), __pointee(type)
{
}

__cxxabiv1::__pbase_type_info::~__pbase_type_info()
{
}

bool __cxxabiv1::__pbase_type_info::__do_catch(const std::type_info *thr_type, void **thr_obj, unsigned outer) const
{
    if(*this == *thr_type)
        return true;

    if(*thr_type == typeid(nullptr))
    {
        if(typeid(*this) == typeid(__pointer_type_info))
        {
            *thr_obj = nullptr;
            return true;
        }
        else if(typeid(*this) == typeid(__pointer_to_member_type_info))
        {
            if(__pointee->__is_function_p())
            {
                using pmf_type = void (__pbase_type_info::*)();
                static const  pmf_type pmf = nullptr;
                *thr_obj = const_cast<pmf_type *>(&pmf);
                return true;
            }
            else
            {
                using pm_type = int __pbase_type_info::*;
                static const pm_type pm = nullptr;
                *thr_obj = const_cast<pm_type *>(&pm);
                return true;
            }
        }
    }

    if(typeid(*this) != typeid(*thr_type))
        return false;

    if(!(outer & 1))
        return false;

    const __pbase_type_info *thrown_type = static_cast<const __pbase_type_info *>(thr_type);

    unsigned tflags = thrown_type->__flags;
    const unsigned fqual_mask = __transaction_safe_mask | __noexcept_mask;
    unsigned throw_fqual = (tflags & fqual_mask);
    unsigned catch_fqual = (__flags & fqual_mask);
    if(throw_fqual & ~catch_fqual)
        tflags &= catch_fqual;
    if(catch_fqual & !throw_fqual)
        return false;
    if(tflags & ~__flags)
        return false;
    if(!(__flags & __const_mask))
        outer &= ~1;
    return __pointer_catch(thrown_type, thr_obj, outer);
}

bool __cxxabiv1::__pbase_type_info::__pointer_catch(const __cxxabiv1::__pbase_type_info *thr_type, void **thr_obj, unsigned outer) const
{
    return __pointee->__do_catch(thr_type->__pointee, thr_obj, outer + 2);
}

__cxxabiv1::__pointer_type_info::__pointer_type_info(const char *n, int quals, const std::type_info *type) :
    __pbase_type_info(n, quals, type)
{
}

__cxxabiv1::__pointer_type_info::~__pointer_type_info()
{
}

bool __cxxabiv1::__pointer_type_info::__is_pointer_p() const
{
    return true;
}

bool __cxxabiv1::__pointer_type_info::__pointer_catch(const __cxxabiv1::__pbase_type_info *thr_type, void **thr_obj, unsigned outer) const
{
    if(outer < 2 && *__pointee == typeid(void))
        return !thr_type->__pointee->__is_function_p();
    return __pbase_type_info::__pointer_catch(thr_type, thr_obj, outer);
}

__cxxabiv1::__pointer_to_member_type_info::__pointer_to_member_type_info(const char *n, int quals, const std::type_info *type, __cxxabiv1::__class_type_info *klass) :
    __pbase_type_info(n, quals, type), __context(klass)
{
}

__cxxabiv1::__pointer_to_member_type_info::~__pointer_to_member_type_info()
{
}

bool __cxxabiv1::__pointer_to_member_type_info::__pointer_catch(const __cxxabiv1::__pbase_type_info *thr_type, void **thr_obj, unsigned outer) const
{
    const __pointer_to_member_type_info *thrown_type = static_cast<const __pointer_to_member_type_info *>(thr_type);
    if(*__context != *thrown_type->__context)
        return false;
    return __pbase_type_info::__pointer_catch(thrown_type, thr_obj, outer);
}

bool __cxxabiv1::__base_class_type_info::__is_virtual_p() const
{
    return __offset_flags & __virtual_mask;
}

bool __cxxabiv1::__base_class_type_info::__is_public_p() const
{
    return __offset_flags & __public_mask;
}

__PTRDIFF_TYPE__ __cxxabiv1::__base_class_type_info::__offset() const
{
    return static_cast<__PTRDIFF_TYPE__>(__offset_flags) >> __offset_shift;
}

__cxxabiv1::__class_type_info::__class_type_info(const char *n) :
    std::type_info(n)
{
}

__cxxabiv1::__class_type_info::~__class_type_info()
{
}

bool __cxxabiv1::__class_type_info::__do_upcast(const __cxxabiv1::__class_type_info *target, void **obj_ptr) const
{
    __upcast_result result (__vmi_class_type_info::__flags_unknown_mask);
    __do_upcast(target, *obj_ptr, result);
    if(!contained_public_p(result.part2dst))
        return false;
    *obj_ptr = const_cast<void *>(result.dst_ptr);
    return true;
}

bool __cxxabiv1::__class_type_info::__do_catch(const std::type_info *thr_type, void **thr_obj, unsigned outer) const
{
    if(*this == *thr_type)
        return true;
    if(outer >= 4)
        return false;
    return thr_type->__do_upcast(this, thr_obj);
}

bool __cxxabiv1::__class_type_info::__do_upcast(const __cxxabiv1::__class_type_info *target, void *obj_ptr, __cxxabiv1::__class_type_info::__upcast_result &result) const
{
    if(*this == *target)
    {
        result.dst_ptr = obj_ptr;
        result.base_type = nonvirtual_base_type;
        result.part2dst = __contained_public;
        return true;
    }
    return false;
}

__cxxabiv1::__class_type_info::__sub_kind __cxxabiv1::__class_type_info::__find_public_src(__PTRDIFF_TYPE__ src2dst, const void *obj_ptr, const __cxxabiv1::__class_type_info *src_type, const void *src_ptr) const
{
    if(src2dst >= 0) return adjust_pointer<void>(obj_ptr, src2dst) == src_ptr ? __contained_public : __not_contained;
    if(src2dst == -2) return __not_contained;
    return __do_find_public_src(src2dst, obj_ptr, src_type, src_ptr);
}

bool __cxxabiv1::__class_type_info::__do_dyncast(__PTRDIFF_TYPE__ src2dst, __cxxabiv1::__class_type_info::__sub_kind access_path, const __cxxabiv1::__class_type_info *dst_type, const void *obj_ptr, const __cxxabiv1::__class_type_info *src_type, const void *src_ptr, __cxxabiv1::__class_type_info::__dyncast_result &result) const
{
    if(obj_ptr == src_ptr && *this == *src_type)
    {
        result.whole2src = access_path;
        return false;
    }
    if(*this == *dst_type)
    {
        result.dst_ptr = obj_ptr;
        result.whole2dst = access_path;
        result.dst2src = __not_contained;
        return false;
    }
    return false;
}

__cxxabiv1::__class_type_info::__sub_kind __cxxabiv1::__class_type_info::__do_find_public_src(__PTRDIFF_TYPE__ src2dst, const void *obj_ptr, const __cxxabiv1::__class_type_info *src_type, const void *src_ptr) const
{
    if(src_ptr == obj_ptr)
        return __contained_public;
    return __not_contained;
}

__cxxabiv1::__class_type_info::__upcast_result::__upcast_result(int d) :
    dst_ptr(nullptr), part2dst(__unknown), src_details(d), base_type(nullptr)
{
}

__cxxabiv1::__class_type_info::__dyncast_result::__dyncast_result(int details) :
    dst_ptr(nullptr), whole2dst(__unknown), whole2src(__unknown), whole_details(details)
{
}

__cxxabiv1::__si_class_type_info::__si_class_type_info(const char *n, const __cxxabiv1::__class_type_info *base) :
    __class_type_info(n), __base_type(base)
{
}

__cxxabiv1::__si_class_type_info::~__si_class_type_info()
{
}

bool __cxxabiv1::__si_class_type_info::__do_dyncast(__PTRDIFF_TYPE__ src2dst, __cxxabiv1::__class_type_info::__sub_kind access_path, const __cxxabiv1::__class_type_info *dst_type, const void *obj_ptr, const __cxxabiv1::__class_type_info *src_type, const void *src_ptr, __cxxabiv1::__class_type_info::__dyncast_result &result) const
{
    if(*this == *dst_type)
    {
        result.dst_ptr = obj_ptr;
        result.whole2dst = access_path;
        if(src2dst >= 0) result.dst2src = adjust_pointer<void>(obj_ptr, src2dst) == src_ptr ? __contained_public : __not_contained;
        else if(src2dst == -2) result.dst2src = __not_contained;
        return false;
    }
    if(obj_ptr == src_ptr && *this == *src_type)
    {
        result.whole2src = access_path;
        return false;
    }
    return __base_type->__do_dyncast(src2dst, access_path, dst_type, obj_ptr, src_type, src_ptr, result);
}

__cxxabiv1::__class_type_info::__sub_kind __cxxabiv1::__si_class_type_info::__do_find_public_src(__PTRDIFF_TYPE__ src2dst, const void *obj_ptr, const __cxxabiv1::__class_type_info *src_type, const void *src_ptr) const
{
    if(src_ptr == obj_ptr && *this == *src_type)
        return __contained_public;
    return __base_type->__do_find_public_src(src2dst, obj_ptr, src_type, src_ptr);
}

bool __cxxabiv1::__si_class_type_info::__do_upcast(const __cxxabiv1::__class_type_info *target, void *obj_ptr, __cxxabiv1::__class_type_info::__upcast_result &result) const
{
    if(__class_type_info::__do_upcast(target, obj_ptr, result))
        return true;
    return __base_type->__do_upcast(target, obj_ptr, result);
}

__cxxabiv1::__vmi_class_type_info::__vmi_class_type_info(const char *n, int flags) :
    __class_type_info(n), __flags(flags), __base_count(0)
{
}

__cxxabiv1::__vmi_class_type_info::~__vmi_class_type_info()
{
}

bool __cxxabiv1::__vmi_class_type_info::__do_dyncast(__PTRDIFF_TYPE__ src2dst, __cxxabiv1::__class_type_info::__sub_kind access_path, const __cxxabiv1::__class_type_info *dst_type, const void *obj_ptr, const __cxxabiv1::__class_type_info *src_type, const void *src_ptr, __cxxabiv1::__class_type_info::__dyncast_result &result) const
{
    if(result.whole_details & __flags_unknown_mask)
        result.whole_details = __flags;

    if(obj_ptr == src_ptr && *this == *src_type)
    {
        result.whole2src = access_path;
        return false;
    }
    if(*this == *dst_type)
    {
        result.dst_ptr = obj_ptr;
        result.whole2dst = access_path;
        if(src2dst >= 0) result.dst2src = adjust_pointer <void> (obj_ptr, src2dst) == src_ptr ? __contained_public : __not_contained;
        else if(src2dst == -2) result.dst2src = __not_contained;
        return false;
    }

    const void *dst_cand = nullptr;
    if(src2dst >= 0) dst_cand = adjust_pointer<void>(src_ptr, -src2dst);
    bool first_pass = true;
    bool skipped = false;

    bool result_ambig = false;
again:
    for(__SIZE_TYPE__ i = __base_count; i--;)
    {
        __dyncast_result result2(result.whole_details);
        void const *base = obj_ptr;
        __sub_kind base_access = access_path;
        __PTRDIFF_TYPE__ offset = __base_info[i].__offset();
        bool is_virtual = __base_info[i].__is_virtual_p();

        if(is_virtual) base_access = __sub_kind(base_access | __contained_virtual_mask);
        base = convert_to_base(base, is_virtual, offset);

        if(dst_cand)
        {
            bool skip_on_first_pass = base > dst_cand;
            if(skip_on_first_pass == first_pass)
            {
                skipped = true;
                continue;
            }
        }

        if(!__base_info[i].__is_public_p ())
        {
            if(src2dst == -2 && !(result.whole_details & (__non_diamond_repeat_mask | __diamond_shaped_mask)))
                continue;
            base_access = __sub_kind(base_access & ~__contained_public_mask);
        }

        bool result2_ambig = __base_info[i].__base_type->__do_dyncast(src2dst, base_access, dst_type, base, src_type, src_ptr, result2);
        result.whole2src = __sub_kind(result.whole2src | result2.whole2src);
        if(result2.dst2src == __contained_public || result2.dst2src == __contained_ambig)
        {
            result.dst_ptr = result2.dst_ptr;
            result.whole2dst = result2.whole2dst;
            result.dst2src = result2.dst2src;
            return result2_ambig;
        }

        if(!result_ambig && !result.dst_ptr)
        {
            result.dst_ptr = result2.dst_ptr;
            result.whole2dst = result2.whole2dst;
            result_ambig = result2_ambig;
            if(result.dst_ptr && result.whole2src != __unknown && !(__flags & __non_diamond_repeat_mask))
                return result_ambig;
        }
        else if(result.dst_ptr && result.dst_ptr == result2.dst_ptr)
            result.whole2dst = __sub_kind(result.whole2dst | result2.whole2dst);
        else if((result.dst_ptr != 0 && result2.dst_ptr != 0) || (result.dst_ptr != 0 && result2_ambig) || (result2.dst_ptr != 0 && result_ambig))
        {
            __sub_kind new_sub_kind = result2.dst2src;
            __sub_kind old_sub_kind = result.dst2src;

            if(contained_p(result.whole2src) && (!virtual_p(result.whole2src) || !(result.whole_details & __diamond_shaped_mask)))
            {
                if(old_sub_kind == __unknown) old_sub_kind = __not_contained;
                if(new_sub_kind == __unknown) new_sub_kind = __not_contained;
            }
            else
            {
                if(old_sub_kind >= __not_contained)
                    ; // do nohing
                else if(contained_p(new_sub_kind) && (!virtual_p(new_sub_kind) || !(__flags & __diamond_shaped_mask)))
                    old_sub_kind = __not_contained;
                else old_sub_kind = dst_type->__find_public_src(src2dst, result.dst_ptr, src_type, src_ptr);

                if(new_sub_kind >= __not_contained)
                    ; // do nothing
                else if(contained_p(old_sub_kind) && (!virtual_p(old_sub_kind) || !(__flags & __diamond_shaped_mask)))
                    new_sub_kind = __not_contained;
                else new_sub_kind = dst_type->__find_public_src(src2dst, result2.dst_ptr, src_type, src_ptr);
            }

            if(contained_p(__sub_kind(new_sub_kind ^ old_sub_kind)))
            {
                if(contained_p(new_sub_kind))
                {
                    result.dst_ptr = result2.dst_ptr;
                    result.whole2dst = result2.whole2dst;
                    result_ambig = false;
                    old_sub_kind = new_sub_kind;
                }
                result.dst2src = old_sub_kind;
                if(public_p(result.dst2src)) return false;
                if(!virtual_p(result.dst2src)) return false;
            }
            else if(contained_p(__sub_kind(new_sub_kind & old_sub_kind)))
            {
                result.dst_ptr = nullptr;
                result.dst2src = __contained_ambig;
                return true;
            }
            else
            {
                result.dst_ptr = nullptr;
                result.dst2src = __not_contained;
                result_ambig = true;
            }
        }

        if(result.whole2src == __contained_private)
            return result_ambig;
    }

    if(skipped && first_pass)
    {
        first_pass = false;
        goto again;
    }

    return result_ambig;
}

__cxxabiv1::__class_type_info::__sub_kind __cxxabiv1::__vmi_class_type_info::__do_find_public_src(__PTRDIFF_TYPE__ src2dst, const void *obj_ptr, const __cxxabiv1::__class_type_info *src_type, const void *src_ptr) const
{
    if(obj_ptr == src_ptr && *this == *src_type)
        return __contained_public;

    for(__SIZE_TYPE__ i = __base_count; i--;)
    {
        if(!__base_info[i].__is_public_p())
            continue;

        const void *base = obj_ptr;
        __PTRDIFF_TYPE__ offset = __base_info[i].__offset();
        bool is_virtual = __base_info[i].__is_virtual_p();

        if(is_virtual)
        {
            if(src2dst == -3)
                continue;
        }
        base = convert_to_base(base, is_virtual, offset);

        __sub_kind base_kind = __base_info[i].__base_type->__do_find_public_src(src2dst, base, src_type, src_ptr);
        if(contained_p(base_kind))
        {
            if(is_virtual) base_kind = __sub_kind(base_kind | __contained_virtual_mask);
            return base_kind;
        }
    }

    return __not_contained;
}

bool __cxxabiv1::__vmi_class_type_info::__do_upcast(const __cxxabiv1::__class_type_info *target, void *obj_ptr, __cxxabiv1::__class_type_info::__upcast_result &result) const
{
    if(__class_type_info::__do_upcast(target, obj_ptr, result))
        return true;

    int src_details = result.src_details;
    if(src_details & __flags_unknown_mask)
        src_details = __flags;

    for(__SIZE_TYPE__ i = __base_count; i--;)
    {
        __upcast_result result2(src_details);
        const void *base = obj_ptr;
        __PTRDIFF_TYPE__ offset = __base_info[i].__offset();
        bool is_virtual = __base_info[i].__is_virtual_p();
        bool is_public = __base_info[i].__is_public_p();

        if(!is_public && !(src_details & __non_diamond_repeat_mask))
            continue;

        if(base) base = convert_to_base(base, is_virtual, offset);

        if(__base_info[i].__base_type->__do_upcast(target, (void *)base, result2))
        {
            if(result2.base_type == nonvirtual_base_type && is_virtual)
                result2.base_type = __base_info[i].__base_type;
            if(contained_p(result2.part2dst) && !is_public)
                result2.part2dst = __sub_kind(result2.part2dst & ~__contained_public_mask);

            if(!result.base_type)
            {
                result = result2;
                if(!contained_p(result.part2dst))
                    return true;

                if(result.part2dst & __contained_public_mask)
                {
                    if(!(__flags & __non_diamond_repeat_mask))
                        return true;
                }
                else
                {
                    if(!virtual_p(result.part2dst))
                        return true;
                    if(!(__flags & __diamond_shaped_mask))
                        return true;
                }
            }
            else if(result.dst_ptr != result2.dst_ptr)
            {
                result.dst_ptr = nullptr;
                result.part2dst = __contained_ambig;
                return true;
            }
            else if(result.dst_ptr)
                result.part2dst = __sub_kind(result.part2dst | result2.part2dst);
            else
            {
                if(result2.base_type == nonvirtual_base_type || result.base_type == nonvirtual_base_type || !(*result2.base_type == *result.base_type))
                {
                    result.part2dst = __contained_ambig;
                    return true;
                }
                result.part2dst = __sub_kind(result.part2dst | result2.part2dst);
            }
        }
    }
    return result.part2dst != __unknown;
}
