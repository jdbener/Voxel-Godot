#include <Godot.hpp>
#include <Spatial.hpp>

using namespace godot;

class SimpleClass: public Spatial {
    GODOT_CLASS(SimpleClass, Spatial);
public:
    SimpleClass();

    // _init must exist as it is called by Godot
    void _init();
    void _ready();

    void test_void_method();
    Variant method(Variant arg);

    static void _register_methods();

    String _name;
    int _value;

    void set_value(int p_value);

    int get_value() const;
};
