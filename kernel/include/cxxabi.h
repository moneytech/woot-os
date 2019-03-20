#pragma once

#include <typeinfo>

namespace __cxxabiv1
{
    class __fundamental_type_info : public std::type_info
    {
    public:
        explicit __fundamental_type_info(const char *n);
        virtual ~__fundamental_type_info();
    };

    class __array_type_info : public std::type_info
    {
    public:
        explicit __array_type_info(const char *n);
        virtual ~__array_type_info();
    };

    class __function_type_info : public std::type_info
    {
    public:
        explicit __function_type_info(const char *n);
        virtual ~__function_type_info();
    protected:
        virtual bool __is_function_p() const;
    };

    class __enum_type_info : public std::type_info
    {
    public:
        explicit __enum_type_info(const char *n);
        virtual ~__enum_type_info();
    };

    class __pbase_type_info : public std::type_info
    {
    public:
        unsigned int __flags;
        const std::type_info* __pointee;

        explicit __pbase_type_info(const char *n, int quals, const std::type_info *type);
        virtual ~__pbase_type_info();

        enum __masks
        {
            __const_mask = 0x1,
            __volatile_mask = 0x2,
            __restrict_mask = 0x4,
            __incomplete_mask = 0x8,
            __incomplete_class_mask = 0x10,
            __transaction_safe_mask = 0x20,
            __noexcept_mask = 0x40
        };
    protected:
        __pbase_type_info(const __pbase_type_info &);
        __pbase_type_info &operator =(const __pbase_type_info &);

        virtual bool __do_catch(const type_info *thr_type, void **thr_obj, unsigned outer) const;
        virtual bool __pointer_catch(const __pbase_type_info *thr_type, void **thr_obj, unsigned outer) const;
    };

    class __pointer_type_info : public __pbase_type_info
    {
    public:
        explicit __pointer_type_info(const char *n, int quals, const std::type_info *type);
        virtual ~__pointer_type_info();
    protected:
        virtual bool __is_pointer_p() const;
        virtual bool __pointer_catch(const __pbase_type_info *thr_type, void **thr_obj, unsigned outer) const;
    };

    class __class_type_info;

    class __pointer_to_member_type_info : public __pbase_type_info
    {
    public:
        __class_type_info *__context;
        explicit __pointer_to_member_type_info(const char *n, int quals, const std::type_info *type, __class_type_info *klass);
        virtual ~__pointer_to_member_type_info();
    protected:
        __pointer_to_member_type_info(const __pointer_to_member_type_info &);
        __pointer_to_member_type_info &operator =(const __pointer_to_member_type_info &);

        virtual bool __pointer_catch(const __pbase_type_info *thr_type, void **thr_obj, unsigned outer) const;
    };

    class __base_class_type_info
    {
    public:
        const __class_type_info *__base_type;
        __INTPTR_TYPE__ __offset_flags;

        enum __offset_flags_masks
        {
            __virtual_mask = 0x1,
            __public_mask = 0x2,
            __hwm_bit = 2,
            __offset_shift = 8
        };

        bool __is_virtual_p() const;
        bool __is_public_p() const;
        __PTRDIFF_TYPE__ __offset() const;
    };

    class __vmi_class_type_info;
#define VMI_CLASS_TYPE_INFO_FLAGS_UNKNOWN_MASK 0x10

    class __class_type_info : public std::type_info
    {
    public:
        explicit __class_type_info(const char *n);
        virtual ~__class_type_info();

        enum __sub_kind
        {
            __unknown = 0,
            __not_contained,
            __contained_ambig,
            __contained_virtual_mask = __base_class_type_info::__virtual_mask,
            __contained_public_mask = __base_class_type_info::__public_mask,
            __contained_mask = 1 << __base_class_type_info::__hwm_bit,
            __contained_private = __contained_mask,
            __contained_public = __contained_mask | __contained_public_mask
        };

        struct __upcast_result
        {
            const void *dst_ptr;
            __sub_kind part2dst;
            int src_details;
            const __class_type_info *base_type;
            __upcast_result(int d);
        };

        struct __dyncast_result
        {
            const void *dst_ptr;
            __sub_kind whole2dst;
            __sub_kind whole2src;
            __sub_kind dst2src;
            int whole_details;
            __dyncast_result(int details = VMI_CLASS_TYPE_INFO_FLAGS_UNKNOWN_MASK);
        protected:
            __dyncast_result(const __dyncast_result &);
            __dyncast_result &operator =(const __dyncast_result &);
        };
    protected:
        virtual bool __do_upcast(const __class_type_info *target, void **obj_ptr) const;
        virtual bool __do_catch(const type_info *thr_type, void **thr_obj, unsigned outer) const;
    public:
        virtual bool __do_upcast(const __class_type_info *target, void *obj_ptr, __upcast_result &result) const;
        __sub_kind __find_public_src(__PTRDIFF_TYPE__ src2dst, const void *obj_ptr, const __class_type_info *src_type, const void *src_ptr) const;
        virtual bool __do_dyncast(__PTRDIFF_TYPE__ src2dst, __sub_kind access_path, const __class_type_info *dst_type, const void *obj_ptr, const __class_type_info *src_type, const void *src_ptr, __dyncast_result &result) const;
        virtual __sub_kind __do_find_public_src(__PTRDIFF_TYPE__ src2dst, const void *obj_ptr, const __class_type_info *src_type, const void *src_ptr) const;
    };

    class __si_class_type_info : public __class_type_info
    {
    public:
        const __class_type_info *__base_type;
        explicit __si_class_type_info(const char *n, const __class_type_info *base);
        virtual ~__si_class_type_info();
    protected:
        __si_class_type_info(const __si_class_type_info &);
        __si_class_type_info &operator =(const __si_class_type_info &);

        virtual bool __do_dyncast(__PTRDIFF_TYPE__ src2dst, __sub_kind access_path, const __class_type_info *dst_type, const void *obj_ptr, const __class_type_info *src_type, const void *src_ptr, __dyncast_result &result) const;
        virtual __sub_kind __do_find_public_src(__PTRDIFF_TYPE__ src2dst, const void *obj_ptr, const __class_type_info *src_type, const void *src_ptr) const;
        virtual bool __do_upcast(const __class_type_info *target, void *obj_ptr, __upcast_result &result) const;
    };

    class __vmi_class_type_info : public __class_type_info
    {
    public:
        unsigned int __flags;
        unsigned int __base_count;
        __base_class_type_info __base_info[1];

        explicit __vmi_class_type_info(const char *n, int flags);
        virtual ~__vmi_class_type_info();

        enum __flags_masks
        {
            __non_diamond_repeat_mask = 0x1,
            __diamond_shaped_mask = 0x2,
            __flags_unknown_mask = VMI_CLASS_TYPE_INFO_FLAGS_UNKNOWN_MASK
        };

    protected:
        virtual bool __do_dyncast(__PTRDIFF_TYPE__ src2dst, __sub_kind access_path, const __class_type_info *dst_type, const void *obj_ptr, const __class_type_info *src_type, const void *src_ptr, __dyncast_result &result) const;
        virtual __sub_kind __do_find_public_src(__PTRDIFF_TYPE__ src2dst, const void *obj_ptr, const __class_type_info *src_type, const void *src_ptr) const;
        virtual bool __do_upcast(const __class_type_info *target, void *obj_ptr, __upcast_result &result) const;
    };
}
