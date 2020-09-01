// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*

    This example program shows how to find frontal human faces in an image and
    estimate their pose.  The pose takes the form of 68 landmarks.  These are
    points on the face such as the corners of the mouth, along the eyebrows, on
    the eyes, and so forth.  
    


    The face detector we use is made using the classic Histogram of Oriented
    Gradients (HOG) feature combined with a linear classifier, an image pyramid,
    and sliding window detection scheme.  The pose estimator was created by
    using dlib's implementation of the paper:
       One Millisecond Face Alignment with an Ensemble of Regression Trees by
       Vahid Kazemi and Josephine Sullivan, CVPR 2014
    and was trained on the iBUG 300-W face landmark dataset (see
    https://ibug.doc.ic.ac.uk/resources/facial-point-annotations/):  
       C. Sagonas, E. Antonakos, G, Tzimiropoulos, S. Zafeiriou, M. Pantic. 
       300 faces In-the-wild challenge: Database and results. 
       Image and Vision Computing (IMAVIS), Special Issue on Facial Landmark Localisation "In-The-Wild". 2016.
    You can get the trained model file from:
    http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2.
    Note that the license for the iBUG 300-W dataset excludes commercial use.
    So you should contact Imperial College London to find out if it's OK for
    you to use this model file in a commercial product.


    Also, note that you can train your own models using dlib's machine learning
    tools.  See train_shape_predictor_ex.cpp to see an example.

    


    Finally, note that the face detector is fastest when compiled with at least
    SSE2 instructions enabled.  So if you are using a PC with an Intel or AMD
    chip then you should enable at least SSE2 instructions.  If you are using
    cmake to compile this program you can enable them by using one of the
    following commands when you create the build project:
        cmake path_to_dlib_root/examples -DUSE_SSE2_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_SSE4_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_AVX_INSTRUCTIONS=ON
    This will set the appropriate compiler options for GCC, clang, Visual
    Studio, or the Intel compiler.  If you are using another compiler then you
    need to consult your compiler's manual to determine how to enable these
    instructions.  Note that AVX is the fastest but requires a CPU from at least
    2011.  SSE4 is the next fastest and is supported by most current machines.  
*/


#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

// ----------------------------------------------------------------------------------------

int main(int argc, char** argv)
{  
    //try
    //{
        // This example takes in a shape model file and then a list of images to
        // process.  We will take these filenames in as command line arguments.
        // Dlib comes with example images in the examples/faces folder so give
        // those as arguments to this program.

        // We need a face detector.  We will use this to get bounding boxes for
        // each face in an image.
        std::cout << "getting\n";
        dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
        std::cout << "gottem\n";
        // And we also need a shape_predictor.  This is the tool that will predict face
        // landmark positions given an image and face bounding box.  Here we are just
        // loading the model from the shape_predictor_68_face_landmarks.dat file you gave
        // as a command line argument.
        dlib::shape_predictor sp;
        std::cout << "deserializing\n";
        dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> sp;
        std::cout << "done\n";


        dlib::image_window win, win_faces;
        // Loop over all the images provided on the command line.
        std::cout << "processing image " << "a" << std::endl;
        dlib::array2d<dlib::rgb_pixel> img;
        load_image(img, "testR.jpg");
        // Make the image larger so we can detect small faces.
        //pyramid_up(img);

        // Now tell the face detector to give us a list of bounding boxes
        // around all the faces in the image.
        std::vector<dlib::rectangle> dets = detector(img);
        std::cout << "Number of faces detected: " << dets.size() << std::endl;

        // Now we will go ask the shape_predictor to tell us the pose of
        // each face we detected.
        std::vector<dlib::full_object_detection> shapes;
        for (unsigned long j = 0; j < dets.size(); ++j)
        {
            cv::Mat input = cv::imread("testR.jpg");//todo hardcode

            dlib:: full_object_detection shape = sp(img, dets[j]);
            
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
            std::cout << "pog " << angle << std::endl;
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
                if (c.y + c.height> strawberryRotate.rows)
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
            std::cout << strawberry.cols << "," << strawberry.rows << ",    " << mask.cols << "," << mask.rows << std::endl;
            

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
            cv::imwrite("result.jpg", result);

            for (unsigned int s = 0; s < shape.num_parts(); s++)
            {
                std::cout << ", " << shape.part(s);
            }
            std::cout << std::endl;
            // You get the idea, you can get all the face part locations if
            // you want them.  Here we just store them in shapes so we can
            // put them on the screen.
            shapes.push_back(shape);
        }


        // Now let's view our face poses on the screen.


        win.clear_overlay();
        win.set_image(img);
        win.add_overlay(render_face_detections(shapes));

        // We can also extract copies of each face that are cropped, rotated upright,
        // and scaled to a standard size as shown here:
        dlib::array<dlib::array2d<dlib::rgb_pixel> > face_chips;
        extract_image_chips(img, get_face_chip_details(shapes), face_chips);
        win_faces.set_image(tile_images(face_chips));

        std::cout << "Hit enter to process the next image..." << std::endl;
        std::cin.get();
    //}
    //catch (std::exception& e)
    //{
    //    std::cout << "\nexception thrown!" << std::endl;
     //   std::cout << e.what() << std::endl;
    //
}

// ----------------------------------------------------------------------------------------

