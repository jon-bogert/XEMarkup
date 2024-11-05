#include <iostream>
#include <string>

#include <XEMarkup/MappingNode.h>
#include <XEMarkup/YAMLFormatter.h>
#include <XEMarkup/JSONFormatter.h>
#include <XEMarkup/BSONFormatter.h>

using namespace xe;

class Vector2 : public IMappable
{
public:
    float x = 0.f;
    float y = 0.f;

    Vector2() = default;
    constexpr Vector2(const float x, const float y) : x(x), y(y) {}

    void Map(MappingNode& node) const override
    {
        node["x"] = x;
        node["y"] = y;
    }

    void Unmap(const MappingNode& node) override
    {
        x = node["x"].As<float>();
        y = node["y"].As<float>();
    }
};

int main (int argc, char* argv[])
{
    MappingNode node;
    Vector2 v;
    v.x = 10.f;
    v.y = 0.5f;
    
    node["player"]["position"] = v;
    node["player"]["name"] = "Name";
    node["player"]["health"] = 89;
    node["enemy-positions"].PushBack(Vector2(400.5f, 3007.736f));
    node["enemy-positions"].PushBack(Vector2(-30.278f, 5555.7362));
    node["enemy-positions"].PushBack(Vector2(3.14159, 78.82916));
    node["enemy-positions"].PushBack(Vector2(25, -35));
    
    YAMLFormatter yaml;
    yaml.SaveFile(node, "test.yaml");
    JSONFormatter json;
    json.SaveFile(node, "test.json");
    json.SetUsePrettyFormat(true);
    json.SaveFile(node, "test_pretty.json");
    BSONFormatter bson;
    bson.SaveFile(node, "test.bin");



    BSONFormatter format;
    node = format.LoadFile("test.bin");
    
    std::cout << node["player"]["position"]["x"].As<float>() << " " << node["player"]["position"]["y"].As<float>() << std::endl;
    std::cout << node["player"]["name"].As<std::string>() << std::endl;
    std::cout << node["player"]["health"].As<float>() << std::endl;
    
    for (MappingNode& pos : node["enemy-positions"])
    {
        std::cout << pos["x"].As<float>() << " " << pos["y"].As<float>() << std::endl;
    }

    return 0;
}
