#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "wS2_32.lib")

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <dlib/opencv/cv_image.h>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <filesystem>

int strawberryize(dlib::frontal_face_detector* detector, dlib::shape_predictor* shapePredictor, cv::Mat *strawberryR, char* inputBuffer, int inputBufferLength, char** outputBuffer)
{
    auto start = std::chrono::high_resolution_clock::now();//todo check for size and memory leaks!!!!!! whats a memory leak
    auto now = start;
    std::cout << "Loading...";
    cv::Mat rawInput(1, inputBufferLength, CV_8UC1, (void*)inputBuffer);
    cv::Mat input = cv::imdecode(rawInput, 1);//todo hardcode
    if (input.data == NULL)
    {
        std::cout << "error decoding image from buffer\n";
        //todo something
    }
    dlib::array2d<dlib::rgb_pixel> img;
    dlib::assign_image(img, dlib::cv_image<dlib::bgr_pixel>(input));
    now = std::chrono::high_resolution_clock::now();
    std::cout << "\rLoaded in " << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() << "ms" << std::endl;
    start = now;

    std::cout << "Finding faces...";
    std::vector<dlib::rectangle> dets = (*detector)(img);
    now = std::chrono::high_resolution_clock::now();
    std::cout << "\rFound " << dets.size() << " faces in " << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() << "ms" << std::endl;
    start = now;

    std::cout << "Applying " << dets.size() << " strawberries..." << std::endl;
    for (unsigned long j = 0; j < dets.size(); ++j)//todo multiple faces
    {
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
        std::vector<unsigned char> outputVector;
        cv::imencode(".jpg", result, outputVector);
        *outputBuffer = new char[outputVector.size()];
        memcpy(*outputBuffer, outputVector.data(), outputVector.size());
        return outputVector.size();
    }
    return 0;
}

void loadDlib(dlib::frontal_face_detector* detector, dlib::shape_predictor* shapePredictor)
{
    try
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "Getting front face detector...";
        *detector = dlib::get_frontal_face_detector();
        auto now = std::chrono::high_resolution_clock::now();
        std::cout << "\rGot front face detector in " << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() << "ms" << std::endl;
        start = now;

        std::cout << "Deserializing training data...";
        dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> *shapePredictor;
        now = std::chrono::high_resolution_clock::now();
        std::cout << "\rDeserialized training data in " << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() << "ms" << std::endl;
        start = now;
    }
    catch (std::exception& e)
    {
        std::cout << "\nexception thrown!" << std::endl;
        std::cout << e.what() << std::endl;
    }
}

#define HOST "localhost"
#define PORT "69"
#define STRAWBERRRY_ROTATE_PNG "strawberry.png"

int main(int argc, char** argv)
{
    std::cout << "Starting Strawberryizer...\n";
    dlib::frontal_face_detector detector;
    dlib::shape_predictor shapePredictor;
    loadDlib(&detector, &shapePredictor);
    cv::Mat strawberryR = cv::imread(STRAWBERRRY_ROTATE_PNG, cv::IMREAD_UNCHANGED);
    std::cout << "Loaded " << STRAWBERRRY_ROTATE_PNG << "\n";
    std::cout << "Initializing network stuff\n";

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
        int recvLength;
        memcpy(&recvLength, recvLengthBuffer, 4);
        char* recvBuffer = new char[recvLength];
        iResult = recv(clientSock, recvBuffer, recvLength, 0);
        if (iResult > 0)
        {
            std::cout << "got " << iResult << " bytes\n";
            if (iResult != recvLength)
            {
                std::cout << "didnt get the right amount, got " << iResult << ", wanted " << recvLength << "\n";
                //todo
            }
            else
            {
                std::cout << "got the goods (" << iResult << ")\n";
                char* strawberryBuffer = nullptr;
                int strawberryBufferSize = strawberryize(&detector, &shapePredictor, &strawberryR, recvBuffer, recvLength, &strawberryBuffer);
                char* outputBuffer;
                int outputBufferSize;
                if (strawberryBufferSize == 0)
                {
                    outputBufferSize = 4;
                    outputBuffer = new char[4];
                    int a = 0;
                    memcpy(outputBuffer, &a, 4);//memset instead???
                }
                else
                {
                    outputBufferSize = 4 + strawberryBufferSize;
                    outputBuffer = new char[outputBufferSize];
                    memcpy(outputBuffer, &strawberryBufferSize, 4);
                    memcpy(outputBuffer + 4, strawberryBuffer, strawberryBufferSize);
                    //delete[] strawberryBuffer;//todo dangerous????
                }
                iResult = send(clientSock, outputBuffer, outputBufferSize, 0);
                if (iResult == SOCKET_ERROR)
                {
                    std::cout << "send failed " << WSAGetLastError();
                    closesocket(sock);
                    WSACleanup();
                    return 1;
                }
                else
                {
                    std::cout << "sent " << iResult << " bytes (file is " << strawberryBufferSize << " bytes long + 4 bytes overhead\n";
                }
                //delete[] outputBuffer;//is this dangerous or somethinhg because its allocated in another method also necessary becasue scope?
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
