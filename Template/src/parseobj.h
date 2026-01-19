#pragma once

#ifndef PARSE_H
#define PARSE_H

#include "common.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <type_traits>

using string = std::string;
using ifstream = std::ifstream;

// This class parses an .obj file to a data structure we can use in the ray tracer.
// File name must be in the "src/obj files" folder.
class Parser
{


	public:
		string filename;

		// colored materials
		
		shared_ptr<Lambertian> red = make_shared<Lambertian>(Vec3(0.8, 0, 0));
		shared_ptr<Lambertian> green = make_shared<Lambertian>(Vec3(0, 0.8, 0));
		shared_ptr<Lambertian> blue = make_shared<Lambertian>(Vec3(0, 0, 0.8));
		shared_ptr<Lambertian> yellow = make_shared<Lambertian>(Vec3(1, 0.7, 0));
		shared_ptr<Lambertian> brown = make_shared<Lambertian>(Vec3(0.075, 0.02, 0));
		shared_ptr<Lambertian> cyan = make_shared<Lambertian>(Vec3(0.085, 0.3, 0.25));
		shared_ptr<Lambertian> pink = make_shared<Lambertian>(Vec3(0.8, 0, 0.3));
		shared_ptr<Lambertian> purple = make_shared<Lambertian>(Vec3(0.3, 0, 0.8));
		

		std::tuple<std::vector<Point3>, std::vector<Vec3>, std::vector<Point3>, std::vector<shared_ptr<Lambertian>>>
		parse(string filename, Point3 pos)
		{
			string dir = lookUpDir();
			ifstream obj(dir + filename);


			std::vector<Point3> vertices; // List of points of the vertices
			std::vector<Vec3> vertex_normals; // Vertex normals --> probably not needed for basic triangulation

			// The x,y,z of the "point" of the face at index i contains the three indices
			// of the points in the "vertices" vector that make up a face at index i
			std::vector<Point3> faces;

			// material of each face
			std::vector<shared_ptr<Lambertian>> materials;

			if (obj.is_open())
			{
				shared_ptr<Lambertian> current_color = cyan;
				while (obj)
				{
					string line = " ";
					std::getline(obj, line);

					if (line.substr(0, 2) == "v ") // Line contains vertex info
					{
						std::istringstream s(line);
						std::vector<string> res;
						string word;

						// Skip the first word, which is "v"
						string skip;
						s >> skip;

						// Get the coordinates from the string
						while (s >> word)
							res.push_back(word);

						auto vec = toPoint3(res);
						vertices.push_back(vec);
					}
					else if (line.substr(0, 6) == "usemtl") // Line contains material info
					{
						cout << "found usemtl" << endl;
						std::istringstream s(line);
						std::vector<string> res;
						string word;

						// Skip the first word, which is "usemtl"
						string skip;
						s >> skip;
						s >> word;

						current_color = getMaterial(word);
						cout << "got color" << endl;
					}
					else if (line.substr(0, 2) == "vn") // Line contains vertex normal info
					{
						std::istringstream s(line);
						std::vector<string> res;
						string word;

						// Skip the first word, which is "vn"
						string skip;
						s >> skip;

						// Get the coordinates from the string
						while (s >> word)
							res.push_back(word);

						auto vec = toVec3(res);
						vertex_normals.push_back(vec);
					}
					else if (line.substr(0, 2) == "f ") // Line contains face info
					{
						std::istringstream s(line);
						std::vector<string> temp;
						std::vector<string> res;
						string word;

						// Skip the first word, which is "f"
						string skip;
						s >> skip;

						// Get the coordinates from the string
						while (s >> word)
							temp.push_back(word);

						// Get the vertex nr out of the string
						// Vertex nr == vertex normal nr
						for (int i = 0; i < temp.size(); i++)
						{
							string w = temp[i];
							std::size_t pos = w.find('/');
							string num = w.substr(0, pos);
							res.push_back(num);
						}

						auto vec = toPoint3(res);
						faces.push_back(vec);
						materials.push_back(current_color);
					}
					else continue; // Other lines can be skipped
				}
			}

			obj.close();

			for (int j = 0; j < vertices.size(); j++)
			{
				vertices[j] += pos;
			}

			//print_vectors(vertices, vertex_normals, faces);

			return { vertices, vertex_normals, faces, materials };
		}

		void print(string out)
		{
			std::cout << out << std::endl;
		}

		Point3 toPoint3(const std::vector<string>& v)
		{
			return Point3(std::stof(v[0]), std::stof(v[1]), std::stof(v[2]));
		}

		Vec3 toVec3(const std::vector<string>& v)
		{
			return Vec3(std::stof(v[0]), std::stof(v[1]), std::stof(v[2]));
		}

		void print_vectors(std::vector<Vec3> vertices, std::vector<Vec3> vertex_normals, std::vector<Vec3> faces)
		{
			print_vector(vertices, "Vertex");
			print_vector(vertex_normals, "Vertex normal");
			print_vector(faces, "Face");
		}

		void print_vector(std::vector<Vec3> vector, string name)
		{
			for (int i = 0; i < vector.size(); i++)
			{
				auto vec3 = vector[i];
				std::cout << name << " " << i << ": " << vec3.x() << " " << vec3.y() << " " << vec3.z() << "\n";
			}
		}

	private:
		string lookUpDir()
		{
			string curr = std::filesystem::current_path().generic_string();

			auto splits = split(curr, "/");

			while (true)
			{
				int index = splits.size() - 1;
				//print(splits[index]);
				if (splits[index] == "Template") break;

				splits.erase(splits.begin() + index);
			}

			std::vector<string> newpathloose;
			for (int i = 0; i < splits.size()-1; i++)
			{
				newpathloose.push_back(splits[i]);
				newpathloose.push_back("/");
			}

			string newpath;
			for (string elem: newpathloose)
				newpath += elem;
			newpath += "Template/src/obj files/";
			return newpath;
		}

		std::vector<string> split(const string& s, string delimiter)
		{
			std::vector<string> elems;
			size_t pos = s.find(delimiter);
			string word;

			string temp = s;
			while (pos != string::npos)
			{
				word = temp.substr(0, pos);
				elems.push_back(word);
				temp.erase(0, pos + delimiter.length());
				pos = temp.find(delimiter);
			}
			elems.push_back(temp);
			return elems;
		}

		shared_ptr<Lambertian> getMaterial(string name)
		{
			shared_ptr<Lambertian> material;
			if (name == "Red") material = red;
			if (name == "Green") material = green;
			if (name == "Blue") material = blue;
			if (name == "Yellow") material = yellow;
			if (name == "Brown") material = brown;
			if (name == "Cyan") material = cyan;
			if (name == "Pink") material = pink;
			if (name == "Purple") material = purple;
			return material;
		}
};

#endif