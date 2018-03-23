#ifndef METAMETHOD_H
#define METAMETHOD_H

#include <string>

#include "variant.h"
#include "metatype.h"
#include "event.h"

using namespace std;

class Object;

class NEXT_LIBRARY_EXPORT MetaMethod {
public:
    typedef Variant            (*InvokeMem)                (Object *, int argc, const Variant *);

    enum MethodType {
        Method                  = 0,
        Signal,
        Slot
    };

    struct Table {
        MethodType              type;
        const char             *name;
        InvokeMem               invoker;
        int                     argc;
        const MetaType::Table **types;
    };

public:
    MetaMethod                 (const Table *table);

    bool                        isValid                     () const;

    const char                 *name                        () const;
    string                      signature                   () const;

    MethodType                  type                        () const;
    MetaType                    returnType                  () const;
    int                         parameterCount              () const;
    MetaType                    parameterType               (int index) const;

    bool                        invoke                      (Object *obj, Variant &returnValue, int argc, const Variant *args) const;

private:
    const Table                *m_pTable;

};

class MethodCallEvent : public Event {
public:
    MethodCallEvent             (int32_t method, Object *senderm, const Variant &args);

    Object                     *sender                      () const;

    int32_t                     method                      () const;

    const Variant              *args                        () const;

protected:
    Object                     *m_pSender;

    int32_t                     m_Method;

    Variant                     m_Args;
};

namespace unpack {
    template<unsigned...> struct indices {};

    template<unsigned N, unsigned... Is>
    struct indices_gen : indices_gen < N - 1, N - 1, Is... > {};

    template<unsigned... Is>
    struct indices_gen<0, Is...> : indices < Is... > {};
}

//Base template for function and methods invocation
template<typename Signature>
struct Invoker;

//Free functions
template<typename Return, typename... Args>
struct Invoker<Return(*)(Args...)> {
    // Workaround for the Visual Studio bug
    typedef Return(*Fun)(Args...);

    inline static int argCount() {
        return sizeof...(Args);
    }

    inline static const MetaType::Table **types() {
        static const MetaType::Table *staticTypes[] = {
            Table<Return>::get(),
            getTable<Args>()...
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static Variant invoke(Object *obj, F f, const Variant *args, unpack::indices<Is...>) {
        return f(args[Is]...); //any_cast<Args>(args[Is])...
    }

    template<Fun fun>
    static Variant invoke(Object *obj, int argc, const Variant *args) {
        if(argc != sizeof...(Args)) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<sizeof...(Args)>());
    }
};

//Free function with no parameters
template<typename Return>
struct Invoker<Return(*)()> {
    typedef Return(*Fun)();

    inline static int argCount() {
        return 0;
    }

    inline static const MetaType::Table **types() {
        static const MetaType::Table *staticTypes[] = {
            Table<Return>::get()
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static Variant invoke(Object *obj, F f, const Variant *args, unpack::indices<Is...>) {
        return f(args[Is]...); // any_cast<Args>(args[Is])...
    }

    template<Fun fun>
    static Variant invoke(Object *obj, int argc, const Variant *args) {
        if(argc != 0) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<0>());
    }
};

//Void free function
template<typename... Args>
struct Invoker<void(*)(Args...)> {
    typedef void(*Fun)(Args...);

    inline static int argCount() {
        return sizeof...(Args);
    }

    inline static const MetaType::Table **types() {
        static const MetaType::Table *staticTypes[] = {
            nullptr,
            getTable<Args>()...
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static Variant invoke(Object *obj, F f, const Variant *args, unpack::indices<Is...>) {
        f(args[Is]...); // any_cast<Args>(args[Is])...
        return Variant();
    }

    template<Fun fun>
    static Variant invoke(Object *obj, int argc, const Variant *args) {
        if(argc != sizeof...(Args)) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<sizeof...(Args)>());
    }
};

//Void free function with no arguments
template<>
struct Invoker<void(*)()> {
    typedef void(*Fun)();

    inline static int argCount() {
        return 0;
    }

    inline static const MetaType::Table **types() {
        static const MetaType::Table *staticTypes[] = {
            nullptr
        };
        return staticTypes;
    }

    template<unsigned... Is>
    inline static Variant invoke(Object *obj, Fun f, const Variant *args, unpack::indices<Is...>) {
        f();
        return Variant();
    }

    template<Fun fun>
    static Variant invoke(Object *obj, int argc, const Variant *args) {
        if(argc != 0) {
            throw std::runtime_error("bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<0>());
    }
};

//Base method template
template<typename Class, typename Return, typename... Args>
struct Invoker<Return(Class::*)(Args...)> {
    typedef Return(Class::*Fun)(Args...);

    inline static int argCount() {
        return sizeof...(Args);
    }

    inline static const MetaType::Table **types() {
        static const MetaType::Table *staticTypes[] = {
            Table<Return>::get(),
            getTable<Args>()...
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static Variant invoke(Object *obj, F f, const Variant *args, unpack::indices<Is...>) {
        return (static_cast<Class *>(obj)->*f)(args[Is]...); // any_cast<Args>(args[Is])...
    }

    template<Fun fun>
    static Variant invoke(Object *obj, int argc, const Variant *args) {
        if(argc != sizeof...(Args)) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<sizeof...(Args)>());
    }
};

//Method with no parameters
template<typename Class, typename Return>
struct Invoker<Return(Class::*)()> {
    typedef Return(Class::*Fun)();

    inline static int argCount() {
        return 0;
    }

    inline static const MetaType::Table **types() {
        static const MetaType::Table *staticTypes[] = {
            Table<Return>::get()
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static Variant invoke(Object *obj, F f, const Variant *, unpack::indices<Is...>) {
        return (static_cast<Class *>(obj)->*f)();
    }

    template<Fun fun>
    static Variant invoke(Object *obj, int argc, const Variant *args) {
        if(argc != 0) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<0>());
    }
};

//Void method
template<typename Class, typename... Args>
struct Invoker<void(Class::*)(Args...)> {
    typedef void(Class::*Fun)(Args...);

    inline static int argCount() {
        return sizeof...(Args);
    }

    inline static const MetaType::Table **types() {
        static const MetaType::Table *staticTypes[] = {
            nullptr,
            getTable<Args>("")... /// \todo: Set name
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static Variant invoke(Object *obj, F f, const Variant *args, unpack::indices<Is...>) {
        (static_cast<Class *>(obj)->*f)(args[Is].value<Args>()...);
        return Variant();
    }

    template<Fun fun>
    static Variant invoke(Object *obj, int argc, const Variant *args) {
        if (argc != sizeof...(Args)) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<sizeof...(Args)>());
    }
};

//Void method with no parameters
template<typename Class>
struct Invoker<void(Class::*)()> {
    typedef void(Class::*Fun)();

    inline static int argCount() {
        return 0;
    }

    inline static const MetaType::Table **types() {
        static const MetaType::Table *staticTypes[] = {
            nullptr
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static Variant invoke(Object *obj, F f, const Variant *args, unpack::indices<Is...>) {
        (static_cast<Class *>(obj)->*f)(args[Is]...); // any_cast<Args>(args[Is])...
        return Variant();
    }

    template<Fun fun>
    static Variant invoke(Object *obj, int argc, const Variant *args) {
        if (argc != 0) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<0>());
    }
};

//Base const method template
template<typename Class, typename Return, typename... Args>
struct Invoker<Return(Class::*)(Args...)const> {
    typedef Return(Class::*Fun)(Args...)const;

    inline static int argCount() {
        return sizeof...(Args);
    }

    inline static const MetaType::Table **types() {
        static const MetaType::Table *staticTypes[] = {
            Table<Return>::get(),
            getTable<Args>()...
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static Variant invoke(Object *obj, F f, const Variant *args, unpack::indices<Is...>) {
        return (const_cast<const Class *>(static_cast<Class *>(obj))->*f)(args[Is]...); // any_cast<Args>(args[Is])...
    }

    template<Fun fun>
    static Variant invoke(Object *obj, int argc, const Variant *args) {
        if (argc != sizeof...(Args)) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<sizeof...(Args)>());
    }
};

//Const method with no parameters
template<typename Class, typename Return>
struct Invoker<Return(Class::*)()const> {
    typedef Return(Class::*Fun)()const;

    inline static int argCount() {
        return 0;
    }

    inline static const MetaType::Table **types() {
        static const MetaType::Table *staticTypes[] = {
            Table<Return>::get()
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static Variant invoke(Object *obj, F f, const Variant *, unpack::indices<Is...>) {
        return (const_cast<const Class *>(static_cast<Class *>(obj))->*f)();
    }

    template<Fun fun>
    static Variant invoke(Object *obj, int argc, const Variant *args) {
        if (argc != 0) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<0>());
    }
};

//Void const method
template<typename Class, typename... Args>
struct Invoker<void(Class::*)(Args...)const> {
    typedef void(Class::*Fun)(Args...)const;

    inline static int argCount() {
        return sizeof...(Args);
    }

    template<typename T>
    inline static MetaType::Table *getTable() {
        return Table<T>::get();
    }

    inline static const MetaType::Table **types() {
        static const MetaType::Table *staticTypes[] = {
            nullptr,
            getTable<Args>()...
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static Variant invoke(Object *obj, F f, const Variant *args, unpack::indices<Is...>) {
        (const_cast<const Class *>(static_cast<Class *>(obj))->*f)(args[Is]...); // any_cast<Args>(args[Is])...
        return Variant();
    }

    template<Fun fun>
    static Variant invoke(Object *obj, int argc, const Variant *args) {
        if(argc != sizeof...(Args)) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<sizeof...(Args)>());
    }
};

//Void const method with no parameters
template<typename Class>
struct Invoker<void(Class::*)()const> {
    typedef void(Class::*Fun)()const;

    inline static int argCount() {
        return 0;
    }

    inline static const MetaType::Table **types() {
        static const MetaType::Table *staticTypes[] = {
            nullptr
        };
        return staticTypes;
    }

    template<typename F, unsigned... Is>
    inline static Variant invoke(Object *obj, F f, const Variant *args, unpack::indices<Is...>) {
        (const_cast<const Class *>(static_cast<Class *>(obj))->*f)(args[Is]...); // any_cast<Args>(args[Is])...
        return Variant();
    }

    template<Fun fun>
    static Variant invoke(Object *obj, int argc, const Variant *args) {
        if(argc != 0) {
            throw std::runtime_error("Bad argument count");
        }
        return invoke(obj, fun, args, unpack::indices_gen<0>());
    }
};

#endif // METAMETHOD_H
