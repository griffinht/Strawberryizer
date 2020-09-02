#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <filesystem>

cv::Mat strawberryize(std::string path, dlib::frontal_face_detector* detector, dlib::shape_predictor* shapePredictor)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto now = start;
    std::cout << "Loading " + path;
    dlib::array2d<dlib::rgb_pixel> img;
    load_image(img, path);
    now = std::chrono::high_resolution_clock::now();
    std::cout << "\rLoaded " + path + " in " << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() << "ms" << std::endl;
    start = now;

    std::cout << "Finding faces...";
    std::vector<dlib::rectangle> dets = (*detector)(img);
    now = std::chrono::high_resolution_clock::now();
    std::cout << "\rFound " << dets.size() << " faces in " << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() << "ms" << std::endl;
    start = now;

    std::cout << "Applying " << dets.size() << " strawberries..." << std::endl;
    for (unsigned long j = 0; j < dets.size(); ++j)//todo multiple faces
    {
        cv::Mat input = cv::imread(path);//todo hardcode

        dlib::full_object_detection shape = (*shapePredictor)(img, dets[j]);

        std::vector<std::vector<cv::Point>> polys;

        std::vector<cv::Point> leftEye;
        for (int s = 36; s < 42; s++)
        {
            dlib::point p = shape.part(s);
            leftEye.push_back(cv::Point(p.x(), p.y()));
        }
        polys.push_back(leftEye);

        std::vector<cv::Point> rightEye;
        for (int s = 42; s < 48; s++)
        {
            dlib::point p = shape.part(s);
            rightEye.push_back(cv::Point(p.x(), p.y()));
        }
        polys.push_back(rightEye);

        std::vector<cv::Point> mouth;
        for (int s = 48; s < 60; s++)
        {
            dlib::point p = shape.part(s);
            mouth.push_back(cv::Point(p.x(), p.y()));
        }
        polys.push_back(mouth);

        cv::Mat mask(input.rows, input.cols, CV_8UC4, cv::Scalar(0, 0, 0, 0));//todo doesnt this do nothing?
        cv::fillPoly(mask, polys, cv::Scalar(255, 255, 255, 255));

        dlib::point left = shape.part(0);
        dlib::point right = shape.part(16);
        dlib::point bottom = shape.part(8);
        dlib::point top = shape.part(27);
        std::cout << "top: " << top.x() << "," << top.y() << " , bottom " << bottom.x() << "," << bottom.y() << std::endl;
        std::cout << right.x() << "-" << left.x() << std::endl;
        float angle = std::atan(((float)(bottom.x() - top.x())) / ((float)(bottom.y() - top.y()))) * 180 / 3.14159f;
        std::cout << (std::atan(top.y() - bottom.y()) / (top.x() - bottom.x())) << std::endl;
        int width = std::sqrt(std::pow(right.x() - left.x(), 2) + std::pow(right.y() - left.y(), 2)) * 1.5;
        int height = std::sqrt(std::pow(bottom.x() - top.x(), 2) + std::pow(bottom.y() - top.y(), 2)) * 2;
        int x = top.x();
        int y = top.y();
        std::cout << angle << " degrees, pos: (" << x << "," << y << "), size: (" << width << "," << height << ")" << std::endl;
        cv::Mat strawberryR = cv::imread("strawberry.png", cv::IMREAD_UNCHANGED);
        cv::resize(strawberryR, strawberryR, cv::Size(width, height), 0, 0, cv::INTER_CUBIC);//todo no inter cubic?
        cv::Mat strawberryRotate(mask.size(), mask.type(), cv::Scalar(0, 0, 0, 0));
        int xO = strawberryRotate.cols / 2 - strawberryR.cols / 2;
        int yO = strawberryRotate.rows / 2 - strawberryR.rows / 2;
        cv::Rect a(xO, yO, strawberryR.cols, strawberryR.rows);
        strawberryR.copyTo(strawberryRotate(a));
        cv::warpAffine(strawberryRotate, strawberryRotate, cv::getRotationMatrix2D(cv::Point2f(strawberryRotate.cols / 2, strawberryRotate.rows / 2), angle, 1.0), strawberryRotate.size());

        cv::Mat strawberry(mask.size(), mask.type(), cv::Scalar(0, 0, 0, 0));
        int xOffset = x - width / 2;
        int yOffset = y - height / 2;
        cv::Rect c(0, 0, strawberry.cols - xOffset, strawberry.rows - yOffset);
        if (xOffset < 0)
        {
            c.x = -xOffset;
            c.width += xOffset;
            xOffset = 0;
            if (c.x + c.width > strawberryRotate.cols)
            {
                c.width -= (c.x + c.width) - strawberryRotate.cols;
            }
        }
        if (yOffset < 0)
        {
            c.y = -yOffset;
            c.height += yOffset;
            yOffset = 0;
            if (c.y + c.height > strawberryRotate.rows)
            {
                c.height -= (c.y + c.height) - strawberryRotate.rows;
            }
        }
        if (xOffset + c.width > strawberry.cols)
        {
            c.width -= (xOffset + c.width) - strawberry.cols;
        }
        if (yOffset + c.height > strawberry.rows)
        {
            c.height -= (yOffset + c.height) - strawberry.rows;
        }
        strawberryRotate(c).copyTo(strawberry(cv::Rect(xOffset, yOffset, c.width, c.height)));

        cv::bitwise_xor(strawberry, mask, strawberry);

        cv::Mat result(input.rows, input.cols, CV_8UC3);
        for (int y = 0; y < result.rows; y++)
        {
            for (int x = 0; x < result.cols; x++)
            {
                float opacity = ((float)strawberry.data[y * strawberry.step + x * strawberry.channels() + 3]) / 255;
                for (int c = 0; c < input.channels(); c++)
                {
                    unsigned char strawberryPixel = strawberry.data[y * strawberry.step + x * strawberry.channels() + c];
                    unsigned char inputPixel = input.data[y * input.step + x * input.channels() + c];
                    result.data[y * input.step + input.channels() * x + c] = inputPixel * (1.0f - opacity) + strawberryPixel * opacity;
                }
            }
        }
        return result;
    }
    return cv::Mat();
}

int main(int argc, char** argv)
{
    try
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "Getting front face detector...";
        dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
        auto now = std::chrono::high_resolution_clock::now();
        std::cout << "\rGot front face detector in " << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() << "ms" << std::endl;
        start = now;

        dlib::shape_predictor shapePredictor;
        std::cout << "Deserializing training data...";
        dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> shapePredictor;
        now = std::chrono::high_resolution_clock::now();
        std::cout << "\rDeserialized training data in " << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() << "ms" << std::endl;
        start = now;

        std::string dir = "testImages";
        for (auto& entry : std::filesystem::directory_iterator(dir))
        {
            std::string path = entry.path().string();
            cv::Mat result = strawberryize(path, &detector, &shapePredictor);
            now = std::chrono::high_resolution_clock::now();
            int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
            std::cout << path << " completed in " << elapsed << "ms" << std::endl;
            std::cout << path;
            cv::imwrite(path.substr(0, path.length() - 3) + std::to_string(elapsed) + ".jpg", result);
            start = now;

        }
        std::cout << "All done" << std::endl;
    }
    catch (std::exception & e)
    {
        std::cout << "\nexception thrown!" << std::endl;
        std::cout << e.what() << std::endl;
    }
}
