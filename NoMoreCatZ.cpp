// NoMoreCatZ.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _TFLITE

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#ifdef _TFLITE
//#include "tensorflow/lite/c/c_api.h"
//#include "tensorflow/lite/c/c_api_internal.h"
#include "tensorflow/lite/c/c_api_experimental.h"
/*#include "absl/memory/memory.h"
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"
#include "tensorflow/lite/delegates/xnnpack/xnnpack_delegate.h"
//#include "tensorflow/lite/examples/label_image/bitmap_helpers.h"
#include "tensorflow/lite/examples/label_image/get_top_n.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/optional_debug_tools.h"
#include "tensorflow/lite/profiling/profiler.h"
#include "tensorflow/lite/string_util.h"
#include "tensorflow/lite/tools/command_line_flags.h"
#include "tensorflow/lite/tools/delegates/delegate_provider.h"
#include "tensorflow/lite/tools/evaluation/utils.h"*/
#pragma comment(lib,"tensorflow2/lib/tensorflowlite_c.dll.if.lib")
#else
#include "tensorflow/c/c_api.h"
#pragma comment(lib,"tensorflow2/lib/tensorflow.lib")
#endif

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#ifdef _DEBUG
#pragma comment(lib,"opencv/lib/opencv_world343d.lib")
#else
#pragma comment(lib,"opencv/lib/opencv_world343.lib")
#endif

using namespace std;
using namespace cv;

void NoOpDeallocator(void* data, size_t a, void* b) {}

BYTE* ParseJson(char* json, unsigned *width, unsigned* height, unsigned *type) {
    BYTE* res = NULL;
    // Get height and width
    sscanf_s(json, "{\"type\":%d,\"width\":%d,\"height\":%d,", type, width, height);
    cerr << "Data: " << *type << ", " << *width << ", " << *height << endl;
    // allocate buffer
    res = (BYTE*)malloc((*width) * (*height) * 4);
    // copy data to buffer
    int ind = 1, pos = 0, dpos = 0;
    char buffer[4] = { 0,0,0,0 };
    while (json[ind] != '{') ind++;
    while (json[ind] != '}') {
        switch (json[ind+1]) {
        case ':': dpos = ind + 2; ind++; break; // start number
        case ',': case '}': strncpy_s(buffer, json+dpos, ind+1-dpos);// end number
            buffer[ind+1-dpos] = 0;
            res[pos++] = atoi(buffer);
            ind++;
            break;
        default: ind++;
        }
    }
    return res;
}


int main(int argc, char* argv[])
{
    _setmode(_fileno(stdin), _O_BINARY);
    cerr << "NoMoreCatZ v0.2.0\n";
    //cout << "Tensorflow " << TF_Version() << endl;
    if (argc < 2) {
        cout << "Usage:" << endl
            << "NoMoreCatz -register [id]\t- Register app for Chrome" << endl
            << "Don't forget to get plugin ID from Chrome!" << endl;
    }
    else {
        string url = argv[1];
        string cpid;
        if (argc >= 3)
            cpid = argv[2];
        string manp2;        
        string manp1 = "{ \"name\": \"com.ttroll.nomorecatz\",\n\"version\" : \"0.2\",\n\"description\" : \"This extension block images with cats using host-based AI\",\n\"path\" : \"";
        if (cpid == "")
            manp2 = "\",\n\"type\": \"stdio\",\n\"allowed_origins\" : [ \"chrome-extension://ikkdkejghellfjlnjmfmllhklcgponce/\" ] ,\n\"manifest_version\" : 2\n}";
        else
            manp2 = "\",\n\"type\": \"stdio\",\n\"allowed_origins\" : [ \"chrome-extension://" + cpid + "/\" ] ,\n\"manifest_version\" : 2\n}";
        //cout << "URL: " << url << endl;
        if (url == "-register") {
            // add to registry
            string manp2;
            string manp1 = "{ \"name\": \"com.ttroll.nomorecatz\",\n\"version\" : \"0.2\",\n\"description\" : \"This extension block images with cats using host-based AI\",\n\"path\" : \"";
            if (cpid == "")
                manp2 = "\",\n\"type\": \"stdio\",\n\"allowed_origins\" : [ \"chrome-extension://ikkdkejghellfjlnjmfmllhklcgponce/\" ] ,\n\"manifest_version\" : 2\n}";
            else
                manp2 = "\",\n\"type\": \"stdio\",\n\"allowed_origins\" : [ \"chrome-extension://" + cpid + "/\" ] ,\n\"manifest_version\" : 2\n}";

            cout << "Registering plugin..." << endl;
            DWORD dwDisposition;
            HKEY hKey1 = NULL;
            RegCreateKeyEx(HKEY_CURRENT_USER,
                TEXT("SOFTWARE\\Google\\Chrome\\NativeMessagingHosts\\com.ttroll.nomorecatz"),
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,//KEY_WRITE,
                NULL,
                &hKey1,
                &dwDisposition);
            char pathBuffer[2048];
            GetCurrentDirectoryA(2047, pathBuffer);
            CharLowerA(pathBuffer);
            string pathMan(pathBuffer);
            pathMan += "\\Plugin\\hostmf.json";
            GetModuleFileNameA(NULL, pathBuffer, 2047);
            CharLowerA(pathBuffer);
            string fullPath(pathBuffer);
            for (int i = 0; i < fullPath.size(); i++) {
                if (fullPath[i] == '\\') {
                    fullPath[i] = '/';
                }
            }
            cout << "Modifying " << pathMan << "...";
            string manfull = manp1 + fullPath + manp2;
            HANDLE hFile = CreateFile(pathMan.c_str(), // Filename
                GENERIC_WRITE,    // Desired access
                FILE_SHARE_WRITE, // Share flags
                NULL,             // Security Attributes
                CREATE_ALWAYS,      // Creation Disposition
                0,                // Flags and Attributes
                NULL);
            if (hFile == INVALID_HANDLE_VALUE)
            {
                printf("Cannot open TestFile\n");
                return 0;
            }
            else
            {
                DWORD dwRet;
                WriteFile(hFile,              // Handle
                    manfull.c_str(), // Data to be written
                    manfull.length(),                 // Size of data, in bytes
                    &dwRet,             // Number of bytes written
                    NULL);             // OVERLAPPED pointer
                CloseHandle(hFile);
                hFile = INVALID_HANDLE_VALUE;
            }
            cout << " Done!" << endl << "Modifying Chrome settings...";
            RegSetValueExA(hKey1, "", 0, REG_SZ, (BYTE*)pathMan.c_str(), pathMan.size());
            cout << " Done!" << endl << "Plugin registered." << endl;
            return 1;
        }
#ifdef _TFLITE
        cerr << "Tensorflow " << TfLiteVersion() << endl;
        //********* Read model
        TfLiteModel* model = TfLiteModelCreateFromFile("./model/model.tflite");
        TfLiteInterpreterOptions* options = TfLiteInterpreterOptionsCreate();
        TfLiteInterpreterOptionsSetNumThreads(options, 16);

        TfLiteInterpreter* interpreter = TfLiteInterpreterCreate(model, options);
    
#else
        cout << "Tensorflow " << TF_Version() << endl;
        //********* Read model
        TF_Graph* Graph = TF_NewGraph();
        TF_Status* Status = TF_NewStatus();

        TF_SessionOptions* SessionOpts = TF_NewSessionOptions();
        TF_Buffer* RunOpts = NULL;

        const char* saved_model_dir = "./model/"; // Path of the model
        const char* tags = "serve"; // default model serving tag; can change in future
        int ntags = 1;

        TF_Session* Session = TF_LoadSessionFromSavedModel(SessionOpts, RunOpts, saved_model_dir, &tags, ntags, Graph, NULL, Status);
        if (TF_GetCode(Status) == TF_OK)
        {
            printf("TF_LoadSessionFromSavedModel OK\n");
        }
        else
        {
            printf("%s", TF_Message(Status));
            return 0;
        }

        int NumInputs = 1;
        TF_Output* Input = (TF_Output*) malloc(sizeof(TF_Output) * NumInputs);

        TF_Output t0 = { TF_GraphOperationByName(Graph, "serving_default_input_tensor"), 0 };
        if (t0.oper == NULL) {
            printf("ERROR: Failed TF_GraphOperationByName serving_default_input\n");
            return 0;
        }

        Input[0] = t0;

        int NumOutputs = 7;
        TF_Output* Output = (TF_Output*) malloc(sizeof(TF_Output) * NumOutputs);

        for (int i = 0; i < 7; i++) {
            TF_Output t2 = { TF_GraphOperationByName(Graph, "StatefulPartitionedCall"), i };
            if (t2.oper == NULL) {
                printf("ERROR: Failed TF_GraphOperationByName StatefulPartitionedCall\n");
                return 0;
            }

            Output[i] = t2;
        }
        // load data....
        TF_Tensor** InputValues = (TF_Tensor**)malloc(sizeof(TF_Tensor*) * NumInputs);
        TF_Tensor** OutputValues = (TF_Tensor**)malloc(sizeof(TF_Tensor*) * NumOutputs);

#endif
        unsigned msgsize, width = 0, height = 0, type = -1;
        DWORD dwRet;
        //while (true) {
        _read(0, &msgsize, 4);
        char* msgbuffer = (char*)malloc(msgsize);
        _read(0, msgbuffer, msgsize);
        BYTE* imgbuf = ParseJson(msgbuffer, &width, &height, &type);
        
        /*HANDLE sFile = CreateFile("c:\\temp\\message.json", // Filename
            GENERIC_WRITE,    // Desired access
            FILE_SHARE_WRITE, // Share flags
            NULL,             // Security Attributes
            CREATE_ALWAYS,      // Creation Disposition
            0,                // Flags and Attributes
            NULL);
        WriteFile(sFile,              // Handle
            &msgsize, // Data to be written
            4,                 // Size of data, in bytes
            &dwRet,             // Number of bytes written
            NULL);
        WriteFile(sFile,              // Handle
            msgbuffer, // Data to be written
            msgsize,                 // Size of data, in bytes
            &dwRet,             // Number of bytes written
            NULL);             // OVERLAPPED pointer
        CloseHandle(sFile);*/
        /*sFile = CreateFile("c:\\temp\\message.bin", // Filename
            GENERIC_WRITE,    // Desired access
            FILE_SHARE_WRITE, // Share flags
            NULL,             // Security Attributes
            CREATE_ALWAYS,      // Creation Disposition
            0,                // Flags and Attributes
            NULL);
        WriteFile(sFile,              // Handle
            imgbuf, // Data to be written
            width*height*4,                 // Size of data, in bytes
            &dwRet,             // Number of bytes written
            NULL);*/
        char buffer2[8192];
        sprintf_s(buffer2, 2047, "{ \"size\" : %d, \"width\" : %d, \"height\" : %d }", msgsize, width, height);
        int outsize2 = strlen(buffer2);
        //_write(1, &outsize2, 4);
        //_write(1, buffer2, outsize2);
        Mat img(height, width, CV_8UC4, imgbuf), tim;
        free(msgbuffer);
        Mat dstp, dst;
        cvtColor(img, dstp, CV_RGBA2RGB);
        //imwrite("c:\\temp\\testimg.png", dstp);
        //float rcratio = (float)*img.cols / *img.rows;
        /*if (img.cols > img.rows)
            resize(dstp, dstp, Size(320, 320 / rcratio));
        else
            resize(dstp, dstp, Size(320 * rcratio, 320));
        if (rcratio != 1.0f)
            copyMakeBorder(dstp, dstp, 0, 320 - dstp.rows, 0, 320-dstp.cols, BORDER_CONSTANT, 0);*/
        cv::resize(dstp, dst, Size(320, 320));
        dst.convertTo(dst, CV_32FC3);// , 1, -127.5f);
        //imwrite("c:\\temp\\testimg_tf.png", dst);
        cv::normalize(dst, dst, -1.0f, 1.0f, NORM_MINMAX);
        
        int ndata = 3 * dst.cols * dst.rows * sizeof(float);
        int ndims = 4;
        int dims[] = { 1,dst.rows,dst.cols,3 };
        sprintf_s(buffer2, 2047, "{ \"size\" : %d, \"width\" : %d, \"height\" : %d }", msgsize, img.cols, img.rows);
        outsize2 = strlen(buffer2);
        //_write(1, &outsize2, 4);
        //_write(1, buffer2, outsize2);

#ifdef _TFLITE
        //TfLiteInterpreterAllocateTensors(interpreter);
        //TfLiteInterpreterResizeInputTensor(interpreter, 0, dims, ndims);
        TfLiteInterpreterAllocateTensors(interpreter);

        TfLiteTensor* input_tensor =
            TfLiteInterpreterGetInputTensor(interpreter, 0);
        TfLiteTensorCopyFromBuffer(input_tensor, dst.ptr(), 
            ndata);

        TfLiteInterpreterInvoke(interpreter);

        int oCount = TfLiteInterpreterGetOutputTensorCount(interpreter);

        float* scores, *types, *boxes;
        float numDetections;
        
        const TfLiteTensor* output_tensor[4];
        for (int i = 0; i < oCount; i++) 
            output_tensor[i] = TfLiteInterpreterGetOutputTensor(interpreter, i);
        /*int odims = TfLiteTensorNumDims(output_tensor);
        int odimsize = TfLiteTensorDim(output_tensor,0),
            odimsize2 = TfLiteTensorDim(output_tensor, 1);*/       
        numDetections = *(float*)TfLiteTensorData(output_tensor[3]);
        scores = (float*)TfLiteTensorData(output_tensor[2]);
        types = (float*)TfLiteTensorData(output_tensor[1]);
        boxes = (float*)TfLiteTensorData(output_tensor[0]);

#else
        TF_Tensor* int_tensor = TF_NewTensor(TF_UINT8, dims, ndims, dst.ptr(), ndata, &NoOpDeallocator, 0);
        //TF_Tensor* int_tensor = TF_NewTensor(TF_UINT8, dims, ndims, dst->imageData, dst->imageSize, &NoOpDeallocator, 0);
        if (int_tensor == NULL) {
            printf("ERROR: Failed TF_NewTensor\n");
        }
        //auto root = tensorflow::Scope::NewRootScope();


        InputValues[0] = int_tensor;
        
        // evaluate
        TF_SessionRun(Session, NULL, Input, InputValues, NumInputs, Output, OutputValues, NumOutputs, NULL, 0, NULL, Status);

        if (TF_GetCode(Status) == TF_OK)
        {
            printf("Session is OK\n");
        }
        else
        {
            printf("%s", TF_Message(Status));
        }

        //  parse results
        float numDetections = ((float*)TF_TensorData(OutputValues[5]))[0];
        float* boxes = (float*)TF_TensorData(OutputValues[1]);
        float* types = (float*)TF_TensorData(OutputValues[2]);
        float* scores = (float*)TF_TensorData(OutputValues[4]);

#endif 
        free(imgbuf);
        char buffer[8192];
        //sprintf_s(buffer3, 2047, "{ \"size\" : %d, \"width\" : %d, \"height\" : %d }", msgsize, img.cols, img.rows);
        sprintf_s(buffer, 2047, "{ \"size\" : %d, \"width\" : %d, \"height\" : %d ,\"result\":", msgsize, width, height);
        int outsize = strlen(buffer);
        
        //char tid[256];
        string outmessage = buffer;
        //outsize = outmessage.length();
        //cerr << "Output size: " << outsize << endl;
        //_write(1, &outsize, 4);
        //_write(1, buffer3, outsize);
        int finResult = 0;
        //Sleep(200);
        //cerr << numDetections << endl;
        for (int i = 0; i < numDetections; i++)
            if (scores[i] > 0.4) {
                if ((int)types[i] == type)
                    finResult = 1;
                //sprintf_s(buffer, 2047, "%d,", (int) types[i]);
                //outmessage += buffer;
                /*printf("Type %f - Score %f\n", types[i], scores[i]);
                printf("  Box: %f,%f - %f,%f\n", boxes[i * 4], boxes[i * 4 + 1], 
                    boxes[i * 4 + 2], boxes[i * 4 + 3]);
                rectangle(img, Point(boxes[i * 4+1] * img.cols, boxes[i * 4] * img.rows),
                    Point(boxes[i * 4 + 3] * img.cols, boxes[i * 4 + 2] * img.rows),
                        Scalar(types[i]*2.8, 255 - types[i]*2.8, types[i] * 2.8), 2);
                sprintf_s(tid, 255, "Type: %d", (int)types[i] + 1);
                putText(img, tid, Point(boxes[i * 4 + 1] * img.cols, boxes[i * 4] * img.rows), FONT_HERSHEY_COMPLEX_SMALL,
                    1, Scalar(0,0,255));*/
            }
            else break;
        sprintf_s(buffer, 2047, "%d}", finResult);
        outmessage += buffer;
        outsize = outmessage.length();
        cerr << "Result: " << finResult << endl;
       /* HANDLE hFile = CreateFile("c:\\temp\\message.out.json", // Filename
            GENERIC_WRITE,    // Desired access
            FILE_SHARE_WRITE, // Share flags
            NULL,             // Security Attributes
            CREATE_ALWAYS,      // Creation Disposition
            0,                // Flags and Attributes
            NULL);
        WriteFile(hFile,              // Handle
            outmessage.c_str(), // Data to be written
            outsize,                 // Size of data, in bytes
            &dwRet,             // Number of bytes written
            NULL);             // OVERLAPPED pointer
        CloseHandle(hFile);*/
        _write(1, &outsize, 4);
        _write(1, outmessage.c_str(), outsize);
        //cvtColor(dst, img, CV_RGB2BGR);
        //string outname = url + ".out.png";
        //cvSaveImage(outname.c_str(), dst);
        //imwrite(outname.c_str(), *img);
        //cvtColor(dst, dst, CV_RGB2BGR);
        //imwrite((url + ".dbg.png").c_str(), dst);

        // deinit
#ifdef _TFLITE
        // Dispose of the model and interpreter objects.
        TfLiteInterpreterDelete(interpreter);
        TfLiteInterpreterOptionsDelete(options);
        TfLiteModelDelete(model);
#else
        TF_DeleteGraph(Graph);
        TF_DeleteSession(Session, Status);
        TF_DeleteSessionOptions(SessionOpts);
        TF_DeleteStatus(Status);
#endif
    }
    return 1;
}

