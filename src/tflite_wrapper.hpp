#pragma once

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "MPU6050.hpp"

#define CLASSIFIED_SHAPES 3

class TFLMicro {
public:
    TFLMicro(const unsigned char tflite_model[], int tensor_arena_size);
    virtual ~TFLMicro();

    int init();

    // template <typename T>
    void input_data(acc_3D<float> *data_arr, size_t size);
    
    void* predict();

    bool is_successful(TfLiteStatus s);
    TfLiteTensor *_input_tensor, *_output_tensor;

private:
    const unsigned char* _tflite_model;
    int _tensor_arena_size;
    uint8_t *_tensor_arena;

    const tflite::Model *_model;
    tflite::MicroInterpreter *_interpreter;
};
