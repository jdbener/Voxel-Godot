#ifndef __CEREAL_GODOT_H__
#define __CEREAL_GODOT_H__
#include <Godot.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <locale>
#include <codecvt>
#include <sstream>

typedef cereal::JSONInputArchive iarchive;
//typedef PortableBinaryInputArchive GodotInputArchive;
typedef cereal::JSONOutputArchive oarchive;
//typedef PortableBinaryOutputArchive GodotOutputArchive;

namespace godot {
    // String (also saves for anything which can be cast to a string)
    template <class Archive>
    inline std::string save_minimal( Archive const&, String const& s ){
        return s.utf8().get_data();
    }
    template <class Archive>
    inline void load_minimal( Archive const&, String& out, std::string const& in ){
        static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        out = converter.from_bytes(in).c_str();
    }

    // Load Vector3 (cast to string)
    template <class Archive>
    inline void load_minimal( Archive const&, Vector3& out, std::string const& in ){
        std::stringstream s(in);
        s >> out.x;
        s.seekg(1L, std::ios::cur);
        s >> out.y;
        s.seekg(1L, std::ios::cur);
        s >> out.z;
    }

    // Load Vector2 (cast to string)
    template <class Archive>
    inline void load_minimal( Archive const&, Vector2& out, std::string const& in ){
        std::stringstream s(in);
        s >> out.x;
        s.seekg(1L, std::ios::cur);
        s >> out.y;
    }
}

#endif // __CEREAL_GODOT_H__
