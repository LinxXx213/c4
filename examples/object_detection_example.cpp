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

#include <c4/lbp.hpp>
#include <c4/jpeg.hpp>
#include <c4/drawing.hpp>
#include <c4/serialize.hpp>
#include <c4/image_dumper.hpp>
#include <c4/meta_data_set.hpp>
#include <c4/classification_metrics.hpp>
#include <c4/object_detection.hpp>

c4::scaling_detector<c4::LBP, 256> load_scaling_detector(const std::string& filepath) {
    std::ifstream fin(filepath, std::ifstream::binary);
    c4::serialize::input_archive in(fin);

    c4::matrix_regression<> mr;
    in(mr);

    c4::window_detector<c4::LBP, 256> wd(mr);

    c4::scaling_detector<c4::LBP, 256> sd(wd, 0.95f);
    return sd;
}

int main(int argc, char* argv[]) {
    try{
        const auto sd = load_scaling_detector("matrix_regression_42_30_stage1_k0_s1_it2k.dat");

        c4::meta_data_set test_meta;
        test_meta.load_vggface2("C:/vggface2/test/", "C:/vggface2/test/loose_landmark_test.csv", 3.0, 100);

        PRINT_DEBUG(test_meta.data.size());

        c4::image_dumper::getInstance().init("", true);

        c4::matrix<uint8_t> img;

        std::vector<c4::image_file_metadata> detections;

        c4::progress_indicator progress(test_meta.data.size());

        for (const auto& t : test_meta.data) {
            c4::read_jpeg(t.filepath, img);

            c4::image_file_metadata ifm;
            ifm.filepath = t.filepath;

            const auto dets = sd.detect(img, 0.99);

            for (const auto& d : dets) {
                ifm.objects.push_back({ d.rect, {} });
                c4::draw_rect(img, d.rect, uint8_t(255), 1);
            }

            for (const auto& g : t.objects) {
                c4::draw_rect(img, g.rect, uint8_t(0), 1);
            }

            detections.push_back(ifm);

            c4::dump_image(img, "fd");

            progress.did_some(1);
        }

        auto res = c4::evaluate_object_detection(test_meta.data, detections, 0.7);

        PRINT_DEBUG(res.recall());
        PRINT_DEBUG(res.precission());
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
