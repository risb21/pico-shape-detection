#pragma once

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#define CLASSIFIED_SHAPES 3

class TFLMicro {
public:
    TFLMicro(const unsigned char tflite_model[], int tensor_arena_size);
    virtual ~TFLMicro();

    int init();
    void* input_data();
    void* predict();
    bool is_op_successful(TfLiteStatus s);

    float input_scale() const;
    int32_t input_zero_point() const;
    TfLiteTensor *_input_tensor, *_output_tensor;
    tflite::MicroInterpreter *_interpreter;

private:
    const unsigned char* _tflite_model;
    int _tensor_arena_size;
    uint8_t *_tensor_arena;

    const tflite::Model *_model;
};
