/* -----------------------------------------------------------------------------

FILE:			Vector3.hpp
DESCRIPTION:	Highly functional Vector3 (and Vector2) with swizzles
NOTES: 			This is a temp system which will be replaced by Godot!

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
Joshua Dahl        2018-12-09         0.0 - Implemented Vector3 (and helper Vector2/Holder)
----------------------------------------------------------------------------- */

#include <ostream>
#include <iostream>

/*
CLASS: Vector2
DESCRIPTION: holds two floating point values
NOTES: only supports the four main math/assignment operators
*/
struct Vector2 {
	// The floating point values being managed
	float x, y;

	// Constructor setting the internal values to the provided values
	Vector2(float x1, float y1) : x(x1), y(y1) {}
	// Constructor defaulting internal values to zero if none are provided
	Vector2() : x(0), y (0) {}

	// Swizzles
	inline float& first() { return x; }
	inline float& second() { return y; }

	// Integer versions of vector
	Vector2 integize(){ return Vector2(int(x), int(y)); }

	// Addition (Scalars and Vector2s)
	void operator+=(float in){
		x += in;
		y += in;
	}
	void operator+=(Vector2 in){
		x += in.x;
		y += in.y;
	}

	// Subtraction (Scalars and Vector2s)
	void operator-=(float in){
		x -= in;
		y -= in;
	}
	void operator-=(Vector2 in){
		x -= in.x;
		y -= in.y;
	}

	// Multiplication (Scalars and Vector2s)
	void operator*=(float in){
		x *= in;
		y *= in;
	}
	void operator*=(Vector2 in){
		x *= in.x;
		y *= in.y;
	}

	// Division (Scalars and Vector2s)
	void operator/=(float in){
		x /= in;
		y /= in;
	}
	void operator/=(Vector2 in){
		x /= in.x;
		y /= in.y;
	}
};

/*
CLASS: Vector2Holder
DESCRIPTION: holds references to two floating point values
NOTES: only supports the four main math/assignment operators
		used as the return type of swizzles in the Vector3 class
*/
struct Vector2Holder {
	// Internal References
	float &x, &y;
	// Constructor links the references with values
	Vector2Holder(float& x1, float& y1) : x(x1), y(y1) {}

	// Swizzles
	inline float& first() { return x; }
	inline float& second() { return y; }

	// Overloads assignment so that we can copy Vector2s
	void operator = (Vector2 vec){
		x = vec.x;
		y = vec.y;
	}

	// Addition (Scalars and Vector2s)
	void operator+=(float in){
		x += in;
		y += in;
	}
	void operator+=(Vector2 in){
		x += in.x;
		y += in.y;
	}

	// Subtraction (Scalars and Vector2s)
	void operator-=(float in){
		x -= in;
		y -= in;
	}
	void operator-=(Vector2 in){
		x -= in.x;
		y -= in.y;
	}

	// Multiplication (Scalars and Vector2s)
	void operator*=(float in){
		x *= in;
		y *= in;
	}
	void operator*=(Vector2 in){
		x *= in.x;
		y *= in.y;
	}

	// Division (Scalars and Vector2s)
	void operator/=(float in){
		x /= in;
		y /= in;
	}
	void operator/=(Vector2 in){
		x /= in.x;
		y /= in.y;
	}
};

/*
CLASS: Vector3
DESCRIPTION: holds three floating point values
NOTES: only supports the four main math/assignment operators
*/
struct Vector3 {
	// Internal Values
	float x, y, z;

	// Constructor which sets the provided values as the internal values
	Vector3(float x1, float y1, float z1) : x(x1), y(y1), z(z1) {}
	// Constructor which sets the internal values to zero
	Vector3() : x(0), y(0), z(0) {}
	// Constructors which take both combinations of a scalar and a vector2
	Vector3(Vector2 xy, float z1) : x(xy.x), y(xy.y), z(z1) {}
	Vector3(float x1, Vector2 yz) : x(x1), y(yz.first()), z(yz.second()) {}

	// Swizzles
	inline float& first() { return x; }
	inline float& second() { return y; }
	inline float& third() { return z; }
	inline Vector2Holder xy(){ return Vector2Holder(x, y); }
	inline Vector2Holder xz(){ return Vector2Holder(x, z); }
	inline Vector2Holder yx(){ return Vector2Holder(y, x); }
	inline Vector2Holder yz(){ return Vector2Holder(y, z); }
	inline Vector2Holder zy(){ return Vector2Holder(y, x); }
	inline Vector2Holder zx(){ return Vector2Holder(y, z); }

	// Integer versions of vector
	Vector3 integize(){ return Vector3(int(x), int(y), int(z)); }

	// Addition (Scalars and Vector3s)
	Vector3 operator+(Vector3 in){
		Vector3 tmp;
		tmp.x = x + in.x;
		tmp.y = y + in.y;
		tmp.z = z + in.z;
		return tmp;
	}
	void operator+=(float in){
		x += in;
		y += in;
		z += in;
	}
	void operator+=(Vector3 in){
		x += in.x;
		y += in.y;
		z += in.z;
	}

	// Subtraction (Scalars and Vector3s)
	Vector3 operator-(Vector3 in){
		Vector3 tmp;
		tmp.x = x - in.x;
		tmp.y = y - in.y;
		tmp.z = z - in.z;
		return tmp;
	}
	void operator-=(float in){
		x -= in;
		y -= in;
		z -= in;
	}
	void operator-=(Vector3 in){
		x -= in.x;
		y -= in.y;
		z -= in.z;
	}

	// Multiplication (Scalars and Vector3s)
	void operator*=(float in){
		x *= in;
		y *= in;
		z *= in;
	}
	void operator*=(Vector3 in){
		x *= in.x;
		y *= in.y;
		z *= in.z;
	}

	// Division (Scalars and Vector3s)
	void operator/=(float in){
		x /= in;
		y /= in;
		z /= in;
	}
	void operator/=(Vector3 in){
		x /= in.x;
		y /= in.y;
		z /= in.z;
	}

	// Equality (and Inequality)
	bool operator!=(const Vector3 in){
		if(x == in.x) return false;
		if(y == in.y) return false;
		if(z == in.z) return false;
		return true;
	}
	bool operator==(const Vector3 in){
		if(int(x) != int(in.x)) return false;
		if(int(y) != int(in.y)) return false;
		if(int(z) != int(in.z)) return false;
		return true;
	}
};

/*
OPERATOR: << (Vector3)
DESCRIPTION: Prints a Vector3 when added in an ostream chain
*/
std::ostream& operator<<(std::ostream& out, Vector3 vec){
	out << "{" << vec.x << ", "
		<< vec.y << ", "
		<< vec.z << "}";
	return out;
}
/*
OPERATOR: << (Vector2Holder)
DESCRIPTION: Prints a Vector2Holder (same way as Vector2) when added in an ostream chain
*/
std::ostream& operator << (std::ostream& out, Vector2Holder vec){
	out << "{" << vec.x << ", "
		<< vec.y << "}";
	return out;
}
/*
OPERATOR: << (Vector2)
DESCRIPTION: Prints a Vector2 when added in an ostream chain
*/
std::ostream& operator << (std::ostream& out, Vector2 vec){
	out << "{" << vec.x << ", "
		<< vec.y << "}";
	return out;
}

// Constant holding a constant representing a null value
const Vector3 NULL_VECTOR(-999, -999, -999);
