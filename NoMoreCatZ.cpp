// NoMoreCatZ.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "tensorflow/c/c_api.h"
//#include <opencv/highgui.h>
//#include <opencv/cv.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
//#include <opencv2/imgproc/types_c.h>

#ifdef _DEBUG
#pragma comment(lib,"opencv/lib/opencv_world343d.lib")
#else
#pragma comment(lib,"opencv/lib/opencv_world343.lib")
#endif

using namespace std;
using namespace cv;

void NoOpDeallocator(void* data, size_t a, void* b) {}

void free_buffer(void* data, size_t length) { free(data); }

/*TF_Buffer* read_file(const char* file) {
    FILE* f = fopen(file, 256, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);                                            

    void* data = malloc(fsize);
    fread(data, fsize, 1, f);
    fclose(f);

    TF_Buffer* buf = TF_NewBuffer();
    buf->data = data;
    buf->length = fsize;
    buf->data_deallocator = free_buffer;
    return buf;
}*/

int main(int argc, char* argv[])
{
    cout << "NoMoreCatZ v0.0.1\n";
    cout << "Tensorflow " << TF_Version() << endl;
    if (argc < 2) {
        cout << "Usage NoMoreCatz <path to image file>\n";
    }
    else {
        string url = argv[1];
        cout << "URL: " << url << endl;
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

        //IplImage* img = cvLoadImage(url.c_str(), CV_LOAD_IMAGE_COLOR);
        //IplImage* dst = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 3);
        //cvCvtColor(img, dst, CV_BGR2RGB);
        Mat img = imread(url.c_str()), dst;
        cvtColor(img, dst, CV_BGR2RGB);

        int ndims = 4;
        //int64_t dims[] = { 1,img->width,img->height,3 };
        int64_t dims[] = { 1,img.rows,img.cols,3 };
        //uint8_t data[] = { 20,20,20,20,20,20,20,20,20,20,20,20 };
        int ndata = 3*img.cols*img.rows*sizeof(uint8_t); // This is tricky, it number of bytes not number of element

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

        cvtColor(dst, img, CV_RGB2BGR);

        printf("Detection results:\n");
        for (int i = 0; i < numDetections; i++)
            if (scores[i] > 0.4) {
                printf("Type %f - Score %f\n", types[i], scores[i]);
                printf("  Box: %f,%f - %f,%f\n", boxes[i * 4], boxes[i * 4 + 1], 
                    boxes[i * 4 + 2], boxes[i * 4 + 3]);
                rectangle(img, Point(boxes[i * 4+1] * img.cols, boxes[i * 4] * img.rows),
                    Point(boxes[i * 4 + 3] * img.cols, boxes[i * 4 + 2] * img.rows),
                        Scalar(types[i]*2.8, 255 - types[i]*2.8, types[i] * 2.8), 2);
            }
            else break;
        
        string outname = url + ".out.png";
        //cvSaveImage(outname.c_str(), dst);
        imwrite(outname.c_str(), img);

        // deinit
        TF_DeleteGraph(Graph);
        TF_DeleteSession(Session, Status);
        TF_DeleteSessionOptions(SessionOpts);
        TF_DeleteStatus(Status);
    }
    return 1;
}

