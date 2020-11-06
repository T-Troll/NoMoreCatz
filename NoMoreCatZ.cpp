// NoMoreCatZ.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "tensorflow/c/c_api.h"

using namespace std;

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
        cout << "Usage here\n";
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
        if (t0.oper == NULL)
            printf("ERROR: Failed TF_GraphOperationByName serving_default_input\n");
        else
            printf("TF_GraphOperationByName serving_default_input is OK\n");

        Input[0] = t0;

        int NumOutputs = 7;
        TF_Output* Output = (TF_Output*) malloc(sizeof(TF_Output) * NumOutputs);

        for (int i = 0; i < 7; i++) {
            TF_Output t2 = { TF_GraphOperationByName(Graph, "StatefulPartitionedCall"), i };
            if (t2.oper == NULL)
                printf("ERROR: Failed TF_GraphOperationByName StatefulPartitionedCall\n");
            else
                printf("TF_GraphOperationByName StatefulPartitionedCall is OK\n");

            Output[i] = t2;
        }
        // load data....
        TF_Tensor** InputValues = (TF_Tensor**)malloc(sizeof(TF_Tensor*) * NumInputs);
        TF_Tensor** OutputValues = (TF_Tensor**)malloc(sizeof(TF_Tensor*) * NumOutputs);

        int ndims = 4;
        int64_t dims[] = { 1,2,2,3 };
        uint8_t data[] = { 20,20,20,20,20,20,20,20,20,20,20,20 };
        int ndata = 3*2*2*sizeof(uint8_t); // This is tricky, it number of bytes not number of element

        TF_Tensor* int_tensor = TF_NewTensor(TF_UINT8, dims, ndims, data, ndata, &NoOpDeallocator, 0);
        if (int_tensor != NULL)
            printf("TF_NewTensor is OK\n");
        else
            printf("ERROR: Failed TF_NewTensor\n");

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
        //float numDetections = ((float*)TF_TensorData(OutputValues[5]))[0];
        float* types = (float*)TF_TensorData(OutputValues[2]);
        float* scores = (float*)TF_TensorData(OutputValues[4]);
        printf("Result Tensor :\n");
        for (int i = 0; i < 10; i++)
            printf("Type %f - %f\n", types[i], scores[i]);

        // deinit
        TF_DeleteGraph(Graph);
        TF_DeleteSession(Session, Status);
        TF_DeleteSessionOptions(SessionOpts);
        TF_DeleteStatus(Status);
    }
    return 1;
}

