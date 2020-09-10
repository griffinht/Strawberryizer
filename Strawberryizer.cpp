#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "wS2_32.lib")

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

cv::Mat strawberryize(cv::Mat *strawberryR, std::string path, dlib::frontal_face_detector* detector, dlib::shape_predictor* shapePredictor)
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
        //std::cout << "top: " << top.x() << "," << top.y() << " , bottom " << bottom.x() << "," << bottom.y() << std::endl;
        //std::cout << right.x() << "-" << left.x() << std::endl;
        float angle = std::atan(((float)(bottom.x() - top.x())) / ((float)(bottom.y() - top.y()))) * 180 / 3.14159f;
        //std::cout << (std::atan(top.y() - bottom.y()) / (top.x() - bottom.x())) << std::endl;
        int width = std::sqrt(std::pow(right.x() - left.x(), 2) + std::pow(right.y() - left.y(), 2)) * 1.5;
        int height = std::sqrt(std::pow(bottom.x() - top.x(), 2) + std::pow(bottom.y() - top.y(), 2)) * 2;
        int x = top.x();
        int y = top.y();
        std::cout << angle << " degrees, pos: (" << x << "," << y << "), size: (" << width << "," << height << ")" << std::endl;
        cv::resize(*strawberryR, *strawberryR, cv::Size(width, height), 0, 0, cv::INTER_CUBIC);//todo no inter cubic?
        cv::Mat strawberryRotate(mask.size(), mask.type(), cv::Scalar(0, 0, 0, 0));
        int xO = strawberryRotate.cols / 2 - (*strawberryR).cols / 2;
        int yO = strawberryRotate.rows / 2 - (*strawberryR).rows / 2;
        cv::Rect a(xO, yO, (*strawberryR).cols, (*strawberryR).rows);
        (*strawberryR).copyTo(strawberryRotate(a));
        cv::warpAffine(strawberryRotate, strawberryRotate, cv::getRotationMatrix2D(cv::Point2f(strawberryRotate.cols / 2, strawberryRotate.rows / 2), angle, 1.0), strawberryRotate.size());
        cv::Mat strawberry(mask.size(), mask.type(), cv::Scalar(0, 0, 0, 0));
        int xOffset = x - strawberry.cols / 2;
        int yOffset = y - strawberry.rows / 2;
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
        std::cout << "left: " << xOffset << ", down: " << yOffset << ", width: " << c.width << ", height: " << c.height << std::endl;
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

int doIt(char* image, int length, char* output)
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
        cv::Mat strawberryR = cv::imread("strawberry.png", cv::IMREAD_UNCHANGED);
        for (auto& entry : std::filesystem::directory_iterator(dir))
        {
            std::string path = entry.path().string();
            cv::Mat result = strawberryize(&strawberryR, path, &detector, &shapePredictor);
            now = std::chrono::high_resolution_clock::now();
            int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
            std::cout << path << " completed in " << elapsed << "ms" << std::endl;
            std::cout << path;
            cv::imwrite(path.substr(0, path.length() - 3) + "done.jpg", result);
            start = now;

        }
        std::cout << "All done" << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "\nexception thrown!" << std::endl;
        std::cout << e.what() << std::endl;
    }
}

#define HOST "localhost"
#define PORT "69"

int main(int argc, char** argv)
{
    std::cout << "Starting Strawberryizer...\n";
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << stderr << "WSAStartup machine broke" << iResult << "\n";
        return 1;
    }

    struct addrinfo *result = 0, hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    //todo necessary?
    iResult = getaddrinfo(NULL, PORT, &hints, &result);
    if (iResult != 0) {
        std::cout << "getaddrinfo failed with error" << iResult << "\n";
        WSACleanup();
        return 1;
    }

    SOCKET sock = INVALID_SOCKET;
    sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock == INVALID_SOCKET)
    {
        std::cout << "error opening socket" << WSAGetLastError();
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = bind(sock, result->ai_addr ,(int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "couldnt bind to port " << PORT << ", got " << WSAGetLastError();
        freeaddrinfo(result);
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(sock, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "listen failed with " << WSAGetLastError();
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    SOCKET clientSock = INVALID_SOCKET;
    clientSock = accept(sock, NULL, NULL);
    if (clientSock == INVALID_SOCKET)
    {
        std::cout << "couldnt accept connection " << WSAGetLastError();
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    closesocket(sock);

    char recvLengthBuffer[4];//todo just make int?
    iResult = recv(clientSock, recvLengthBuffer, 4, 0);
    if (iResult > 0)
    {
        std::cout << "got " << iResult << "bytes \n";
        int recvLength = (int) recvLengthBuffer;
        char* recvBuffer = new char[recvLength];
        iResult = recv(clientSock, recvBuffer, recvLength, 0);
        if (iResult > 0)
        {
            std::cout << "got " << iResult << " bytes\n";
            if (iResult != recvLength)
            {
                std::cout << "didnt get the right amount, got " << iResult << "\n";
                //todo
            }
            else
            {
                std::cout << "got the goods (" << iResult << ")\n";
                char* output;
                int outputLength = doIt(recvBuffer, recvLength, output + 4);
                memset(output, outputLength, 4);
                int sendLength = 4 + outputLength;
                iResult = send(clientSock, output, sendLength, 0);
                if (iResult == SOCKET_ERROR)
                {
                    std::cout << "send failed " << WSAGetLastError();
                    closesocket(sock);
                    WSACleanup();
                    return 1;
                }
                else
                {
                    std::cout << "sent " << iResult << " bytes (file is " << outputLength << " bytes long + 4 bytes overhead\n";
                }
                delete[] output;//is this dangerous or somethinhg because its allocated in another method
            }
        }
        else if (iResult == 0)
        {
            std::cout << "the connection is the close\n";
        }
        else
        {
            std::cout << "error on recv " << iResult << "\n";
        }
    }
    else if (iResult == 0)
    {
        std::cout << "closing" << std::endl;
    }
    else 
    {
        std::cout << "recv failed " << WSAGetLastError();
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    iResult = shutdown(clientSock, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        std::cout << "shutdown failed " << WSAGetLastError();
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    closesocket(clientSock);
}
