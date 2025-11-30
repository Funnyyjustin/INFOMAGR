#ifndef RAYTRACER_OBJPARSER_H
#define RAYTRACER_OBJPARSER_H


#include <vector>

#include "common.h"
#include "triangle.h"



class Object : public Primitive
{
    public:
        std::vector<Triangle> triangles;

        Object(const Point3& pos, shared_ptr<Material> mat)
        : pos(pos), mat(mat) {}


        bool hit(const Ray& r, Interval ray_t, Hit_record& rec) const override
        {
            for (const Triangle& triangle: triangles) triangle.hit(r, ray_t, rec);
        }

    private:
        Point3 pos;
        shared_ptr<Material> mat;

};

#include <iostream>
#include <string>
using namespace std;
class ObjParser
{
    public:
        ObjParser() {}

        Object Parse(const string file, const Point3& pos)
        {
            ifstream objfile (file);
            string line;
            vector<Triangle> mesh;
            while (std::getline ((objfile, line)))
            {
                if (line(0) == 'v') {

                }
            }
        }

    private:
        string file;
        Point3 pos;

        Triangle line_to_triangle(string& line)
        {
            line.erase(0, 2);
            auto elems = split(line, " ");

            auto a = stod(elems(0));
            auto b = stod(elems(1));
            auto c = stod(elems(2));


        }

        vector<string> split(const string& s, const string& delimiter)
        {
            vector<string> tokens;
            size_t pos = 0;
            string token;
            while ((pos = s.find(delimiter)) != string::npos)
            {
                token = s.substr(0, pos);
                tokens.push_back(token);
                s.erase(0, pos + delimiter.length());
            }
            tokens.push_back(s);

            return tokens;
        }

};

#endif //RAYTRACER_OBJPARSER_H