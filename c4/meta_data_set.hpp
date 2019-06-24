//MIT License
//
//Copyright(c) 2019 Alex Kasitskyi
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once

#include <string>
#include <vector>
#include <fstream>

#include "csv.hpp"
#include "json.hpp"
#include "range.hpp"
#include "string.hpp"
#include "geometry.hpp"

namespace c4 {
    struct image_file_metadata {
        std::string filepath;
        std::vector<object_on_image> objects;

        image_file_metadata() = default;
        image_file_metadata(const std::string& filepath, const std::vector<object_on_image>& objects) : filepath(filepath), objects(objects) {}
        image_file_metadata(std::pair<std::string, std::vector<object_on_image>>&& pair) : filepath(pair.first), objects(pair.second) {}
    };

    struct meta_data_set {
        std::vector<image_file_metadata> data;

        static c4::rectangle<int> make_rect_by_landmarks(const std::vector<point<float>>& landmarks, const float scale) {
            const point<float> center = std::accumulate(landmarks.begin(), landmarks.end(), point<float>()) * (1.f / landmarks.size());

            float max_d = 0.f;
            for (auto p : landmarks) {
                max_d = std::max(max_d, dist(p, center));
            }

            const float half_side = max_d * scale;
            const int side = int(2 * half_side + 0.5f);

            return rectangle<int>(int(center.x - half_side + 0.5f), int(center.y - half_side + 0.5f), side, side);
        }

        void load_dlib(const std::string& root, const std::string& labels_filepath, const float rect_scale, const int sample) {
            json data_json;

            std::ifstream train_data_fin(root + labels_filepath);

            train_data_fin >> data_json;

            for (int i : range(data_json["images"].size())) {
                const auto& image_info = data_json["images"][i];

                const std::string filename = image_info["filename"];

                if (!ends_with(to_lower(filename), ".jpg") || i % sample)
                    continue;

                std::vector<object_on_image> objects;

                for (const auto& box : image_info["boxes"]) {
                    object_on_image o;
                    o.rect.x = box["left"];
                    o.rect.y = box["top"];
                    o.rect.w = box["width"];
                    o.rect.h = box["height"];

                    for (const auto& l : box["landmarks"]) {
                        o.landmarks.emplace_back(float(l["x"]), float(l["y"]));
                    }

                    //std::vector<point<float>> main_landmarks;
                    //main_landmarks.push_back(o.landmarks[30]); // nose
                    //main_landmarks.push_back((o.landmarks[36] + o.landmarks[39]) * 0.5f); // left eye
                    //main_landmarks.push_back((o.landmarks[42] + o.landmarks[45]) * 0.5f); // right eye
                    //main_landmarks.push_back((o.landmarks[48]) * 0.5f); // left mouth corner
                    //main_landmarks.push_back((o.landmarks[54]) * 0.5f); // right mouth corner


                    //o.rect = make_rect_by_landmarks(o.landmarks, rect_scale);

                    objects.push_back(o);
                }

                data.emplace_back(root + filename, objects);
            }
        }

        void load_vggface2(const std::string& root, const std::string& labels_filepath, const float rect_scale, const int sample) {
            std::ifstream fin(labels_filepath);

            std::map<std::string, std::vector<object_on_image>> data_set;

            csv csv;
            csv.read(fin);
            for (int i : range(1, isize(csv.data))) {
                const auto& row = csv.data[i];

                object_on_image o;
                o.landmarks.resize(5);
                
                ASSERT_EQUAL(row.size(), o.landmarks.size() * 2 + 1);
                
                const std::string filepath = root + row[0] + ".jpg";

                for (int j : range(o.landmarks)) {
                    o.landmarks[j].x = string_to<float>(row[2 * j + 1]);
                    o.landmarks[j].y = string_to<float>(row[2 * j + 2]);
                }

                o.rect = make_rect_by_landmarks(o.landmarks, rect_scale);

                data_set[filepath].push_back(o);
            }

            int n = isize(data);
            data.insert(data.end(), std::make_move_iterator(data_set.begin()), std::make_move_iterator(data_set.end()));

            if (sample == 1)
                return;
            
            for (int i : range(n, isize(data))) {
                if (i % sample == 0) {
                    data[n++] = std::move(data[i]);
                }
            }

            data.resize(n);
        }
    };
}; // namespace c4
