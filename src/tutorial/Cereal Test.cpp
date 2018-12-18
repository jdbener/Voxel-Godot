#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/json.hpp>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <string>

struct Vector2 {
    int x, y;

    Vector2(int _x, int _y) : x(_x), y(_y) { }
    Vector2() : x(rand() % 100), y(rand() % 100) { }

    template <class Archive>
    void serialize( Archive & ar ) {
        ar ( CEREAL_NVP(x), CEREAL_NVP(y) );
    }
};

struct Vector3: public Vector2 {
    int z;

    Vector3(int _x, int _y, int _z) : Vector2(_x, _y), z(_z) { }
    Vector3() : Vector2(), z(rand() % 100) { }

    template <class Archive>
    void serialize( Archive & ar ) {
        Vector2::serialize(ar);

        ar ( CEREAL_NVP(z) );
    }
};

struct SomeData {
    Vector3 data[10];

    template <class Archive>
    void serialize( Archive & ar ) {
        for(int i = 0; i < 10; i++)
            ar ( cereal::make_nvp("data#" +  std::to_string(i), data[i]) );
    }
};

std::ostream& operator<< (std::ostream& stream, Vector3 vec){
    stream << "{" << vec.x << ", " << vec.y << ", " << vec.z << "}";
    return stream;
}

int main()
{
    srand(time(0));
    {
        std::ofstream os("out.cereal");
        cereal::JSONOutputArchive archive( os );

        SomeData myData;
        archive( myData );
    }

    {
        std::ifstream is("out.cereal");
        cereal::JSONInputArchive inarchive( is );

        SomeData inData;
        inarchive( inData );

        int index = rand() % 10;
        std::cout << index << ": " << inData.data[index] << std::endl;
    }

  system("pause");

  return 0;
}
