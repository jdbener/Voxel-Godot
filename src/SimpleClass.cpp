/*
    Tutorial from:
    https://github.com/GodotNativeTools/godot-cpp
    https://gist.github.com/willnationsdev/437eeaeea2e675c0bea53343f7ecd4cf
*/

#include <Godot.hpp>
#include <Spatial.hpp>
#include <SceneTree.hpp>
#include <Viewport.hpp>

#include "SimpleClass.h"

using namespace godot;

SimpleClass::SimpleClass() { }

// _init must exist as it is called by Godot
void SimpleClass::_init() { }

void SimpleClass::test_void_method(){
    Godot::print("This is a test");
}

Variant SimpleClass::method(Variant arg){
    Variant ret;
    ret = arg;
    return ret;
}

void SimpleClass::_ready(){
    Node* root = get_tree()->get_root()->get_child(0);
    Godot::print(String("Node: ") + root->get_name() + " found");
};

void SimpleClass::_register_methods(){
    register_method("method", &SimpleClass::method);
    register_method("_ready", &SimpleClass::_ready);

    /*
        How to register exports like gdscript, eg.
        export var _name = "SimpleClass"
    */
    register_property<SimpleClass, String>("base/get", &SimpleClass::_name, String("SimpleClass"));
    // alternatively with getter and setter register_methods
    register_property<SimpleClass, int>("base/value", &SimpleClass::set_value, &SimpleClass::get_value, 0);
    /** For registering signal **/
    // register_signal<SimpleClass>("signal_name");
    // register_signal<SimpleClass>("signal_name", "string_argument", GODOT_VARIANT_TYPE_STRING)
}

void SimpleClass::set_value(int p_value){
    _value = p_value;
}

int SimpleClass::get_value() const {
    return _value;
}
