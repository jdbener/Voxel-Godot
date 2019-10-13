/*
    This file implements an output stream linked to Godot's console.
    Author: Joshua "Jdbener" Dahl

    NOTE: Messages are synced to Godot when a '\n' or an std::endl is encounted.
    The last character sent must be one of these in order for the things before it
    to print.
    NOTE: By default string literals are interpeted as char*s... to use unicode
    symbols make sure to start the literal with an L, ex: L"152 μs"
    NOTE: std::flush is not supported since Godot's print function automatically
    ends the line, thus std::flush would produce odd results equivalent to std::endl.
    Use std::endl instead. (std::flush won't cause errors, it just does nothing impactful)
*/

#ifndef _GSTREAM_H_
#define _GSTREAM_H_
#include "Godot.hpp"
#include <sstream>

// Define how Godot Strings get processed by wostreams
std::wostream& operator<<(std::wostream& stream, godot::String& string){
    stream << string.unicode_str();
    return stream;
}
// By defining an rvalue function as well, any datatype with a String() opperator
// is now supported by any wostream
std::wostream& operator<<(std::wostream& stream, godot::String&& string){
    return stream << string;
}

namespace godot {
    // Function which converts anything with << overloaded for wostreams, into a Godot String
    template<class T>
    godot::String convertToString(T& t) {
        std::wstringstream ss;
        ss << t;
        return godot::String(ss.str().c_str());
    }
    // Macro enabling shorthand conversions
    #define c(x) convertToString(x)

    // Original class code from: http://videocortex.io/2017/custom-stream-buffers/
    class godot_streambuf: public std::wstreambuf {
    private:
        // Buffer which is used to store characters before sending them to Godot
        std::wstring buffer;
    public:
        // Function which constucts an instance of the stream buffer
        static godot_streambuf* get_singleton(){
            static godot_streambuf gdbuf = godot_streambuf();
            return &gdbuf;
        }

    protected:
        // Function which processes the characters in the stream as they arrive.
        void storeCharacter(const int_type character){
            // If the character is a newline: send the buffer to Godot, then clear the buffer
            if(character == '\n'){
                godot::String toPrint = buffer.c_str();
                if(toPrint.length() == 0)
                    toPrint = wchar_t(' '); // Print a space to ensure extra newlines happen
                Godot::print(toPrint);
                buffer.clear();
            // Otherwise add the character to the buffer
            } else
                buffer += wchar_t(character);
        }

        // Functions which direct characters to storeCharacter()
        std::streamsize xsputn(const char_type* string, std::streamsize count) override {
            for (int i = 0; i < count; i++)
                storeCharacter(string[i]);
            return count; // returns the number of characters successfully written.
        }
        int_type overflow(int_type character) override {
            storeCharacter(character);
            return 1;
        }
    };

    // Implementaion of the streambuffer
    static std::wostream gout(godot_streambuf::get_singleton());
};

#endif
